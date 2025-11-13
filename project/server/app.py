# server/app.py
import os
import ctypes
import json
import re
from flask import Flask, request, jsonify, render_template
from ctypes import c_int, c_char_p, c_void_p

# ---------------- helpers ----------------
def _project_path(*parts):
    base = os.path.dirname(os.path.abspath(__file__))
    return os.path.normpath(os.path.join(base, *parts))

def _load_library():
    # Try a few likely filenames/locations
    candidates = [
        _project_path('..', 'build', 'libfriend.dylib'),
        _project_path('..', 'build', 'libfriend.so'),
        _project_path('libfriend.dylib'),
        _project_path('libfriend.so'),
    ]
    for p in candidates:
        if os.path.exists(p):
            try:
                lib = ctypes.CDLL(p)
                print("Loaded shared library:", p)
                return lib, p
            except Exception as e:
                print(f"Failed to load {p}: {e}")
    return None, None

lib, libpath = _load_library()

def resolve_symbol(name):
    """
    Try to resolve symbol by trying multiple variants:
      name, _name, name without leading underscore, _name without leading underscore
    Returns function pointer or None.
    """
    if not lib:
        return None
    tries = [name, f"_{name}", name.lstrip('_'), f"_{name.lstrip('_')}"]
    for t in tries:
        try:
            return getattr(lib, t)
        except AttributeError:
            continue
    return None

def _free_string(ptr):
    """Call free function if available. Accepts an integer or c_void_p."""
    if not ptr:
        return
    free_fn = resolve_symbol('api_free_string') or resolve_symbol('_api_free_string')
    if free_fn is None:
        # Nothing to do; best effort
        return
    try:
        free_fn.argtypes = [c_void_p]
        free_fn.restype = None
        # ensure we pass a c_void_p
        if isinstance(ptr, int):
            free_fn(c_void_p(ptr))
        else:
            free_fn(ptr)
    except Exception:
        # swallow - best-effort free
        pass

def _coerce_arg(a):
    """Convert Python arg to ctypes-friendly type for call_str."""
    if isinstance(a, int):
        return c_int(a)
    if isinstance(a, bytes):
        return c_char_p(a)
    if isinstance(a, str):
        return c_char_p(a.encode('utf-8'))
    # fallback to string
    return c_char_p(str(a).encode('utf-8'))

def call_str(fn, *args):
    """
    Call a C function that returns a heap-allocated char* pointer as c_void_p.
    Will decode to Python string and free via api_free_string if available.
    """
    if fn is None:
        raise RuntimeError("function not found")
    # don't set argtypes globally here; just pass converted args so ctypes maps them
    fn.restype = c_void_p
    c_args = tuple(_coerce_arg(a) for a in args)
    res = fn(*c_args)
    if not res:
        return None
    try:
        # obtain bytes and decode
        raw = ctypes.string_at(res)
        s = raw.decode('utf-8', errors='replace')
    finally:
        _free_string(res)
    return s

def call_mixed_str_or_int(fn, *args):
    """Safely handle functions that may return either char* or int.

    Heuristic: set restype=c_void_p, read the raw value; if it's a small integer
    (<4096), treat it as integer return. Otherwise, treat it as char* and decode,
    then free via _api_free_string.
    """
    if fn is None:
        raise RuntimeError("function not found")
    fn.restype = c_void_p
    c_args = tuple(_coerce_arg(a) for a in args)
    res = fn(*c_args)
    if not res:
        raise RuntimeError('null return')
    addr = res if isinstance(res, int) else getattr(res, 'value', None)
    if addr is None:
        # Fallback decode attempt
        try:
            raw = ctypes.string_at(res)
            s = raw.decode('utf-8', errors='replace')
            _free_string(res)
            return s
        except Exception as e:
            raise RuntimeError(f'unknown return type: {e}')
    if addr < 4096:
        return int(addr)
    try:
        raw = ctypes.string_at(addr)
        s = raw.decode('utf-8', errors='replace')
    finally:
        _free_string(addr)
    return s

def try_parse_json(s):
    if s is None:
        return None
    try:
        return json.loads(s)
    except Exception:
        return s

def _to_int(x):
    try:
        return int(x)
    except Exception:
        return None

def _to_float(x):
    try:
        return float(x)
    except Exception:
        return None

def parse_mutual_text(s):
    """Parse mutual recommendation text output into a list of {id, mutuals}.
    Accepts formats like:
      - "id,mutuals" per line
      - "id mutuals"
      - "id: X, mutuals: Y"
      - JSON string (delegated to try_parse_json)
    """
    js = try_parse_json(s)
    if isinstance(js, (list, dict)):
        return js
    if not isinstance(s, str):
        return s
    out = []
    for line in s.splitlines():
        line = line.strip()
        if not line:
            continue
        m = re.search(r"id\s*[:=]\s*(\-?\d+).*?mutuals\s*[:=]\s*(\d+)", line, re.I)
        if m:
            out.append({"id": _to_int(m.group(1)), "mutuals": _to_int(m.group(2))})
            continue
        m = re.match(r"^(\-?\d+)\s*,\s*(\d+)$", line)
        if m:
            out.append({"id": _to_int(m.group(1)), "mutuals": _to_int(m.group(2))})
            continue
        m = re.match(r"^(\-?\d+)\s+(\d+)$", line)
        if m:
            out.append({"id": _to_int(m.group(1)), "mutuals": _to_int(m.group(2))})
            continue
    return out if out else s

def parse_weighted_text(s):
    """Parse weighted recommendation text output into [{id, score, mutuals, shared}].
    Accept formats:
      - "id,score,mutuals,shared"
      - "id score mutuals shared"
      - "id: X, score: S, mutuals: M, shared: J"
      - Fallback: "id,score" or "id score"
      - JSON string (delegated to try_parse_json)
    """
    js = try_parse_json(s)
    if isinstance(js, (list, dict)):
        return js
    if not isinstance(s, str):
        return s
    out = []
    for line in s.splitlines():
        line = line.strip()
        if not line:
            continue
        m = re.search(r"id\s*[:=]\s*(\-?\d+)\b.*?score\s*[:=]\s*([\-+]?\d*\.?\d+)\b.*?(mutuals|mutual|mutual_count)\s*[:=]\s*(\d+)\b.*?(shared\s*interest|shared_interest|shared_interests|shared)\s*[:=]\s*(\d+)", line, re.I)
        if m:
            out.append({
                "id": _to_int(m.group(1)),
                "score": _to_float(m.group(2)),
                "mutuals": _to_int(m.group(4)),
                "shared": _to_int(m.group(6)),
            })
            continue
        m = re.match(r"^(\-?\d+)\s*,\s*([\-+]?\d*\.?\d+)\s*,\s*(\d+)\s*,\s*(\d+)$", line)
        if m:
            out.append({
                "id": _to_int(m.group(1)),
                "score": _to_float(m.group(2)),
                "mutuals": _to_int(m.group(3)),
                "shared": _to_int(m.group(4)),
            })
            continue
        m = re.match(r"^(\-?\d+)\s+([\-+]?\d*\.?\d+)\s+(\d+)\s+(\d+)$", line)
        if m:
            out.append({
                "id": _to_int(m.group(1)),
                "score": _to_float(m.group(2)),
                "mutuals": _to_int(m.group(3)),
                "shared": _to_int(m.group(4)),
            })
            continue
        # fallback to id + score only
        m = re.match(r"^(\-?\d+)\s*,\s*([\-+]?\d*\.?\d+)$", line)
        if m:
            out.append({"id": _to_int(m.group(1)), "score": _to_float(m.group(2))})
            continue
        m = re.match(r"^(\-?\d+)\s+([\-+]?\d*\.?\d+)$", line)
        if m:
            out.append({"id": _to_int(m.group(1)), "score": _to_float(m.group(2))})
            continue
    return out if out else s

# ----------------- python wrappers -----------------
# Each wrapper resolves symbol and sets argtypes/restype properly.
# IMPORTANT: Ensure these match your C exports exactly.

def _api_add_user_py(name: str):
    fn = resolve_symbol('_api_add_user') or resolve_symbol('api_add_user')
    if not fn:
        raise RuntimeError('_api_add_user not found')
    fn.argtypes = [c_char_p]
    fn.restype = c_int
    return int(fn(name.encode('utf-8')))

def _api_add_user_with_id_py(name: str, uid: int):
    # NOTE: C signature is (const char* name, int fixedId)
    fn = resolve_symbol('_api_add_user_with_id') or resolve_symbol('api_add_user_with_id')
    if not fn:
        raise RuntimeError('_api_add_user_with_id not found')
    fn.argtypes = [c_char_p, c_int]
    fn.restype = c_int
    return int(fn(name.encode('utf-8'), c_int(uid)))

def _api_remove_user_py(uid: int):
    fn = resolve_symbol('_api_remove_user') or resolve_symbol('api_remove_user')
    if not fn:
        raise RuntimeError('_api_remove_user not found')
    fn.argtypes = [c_int]
    fn.restype = ctypes.c_bool
    return bool(fn(c_int(uid)))

def _api_add_friend_py(a: int, b: int):
    fn = resolve_symbol('_api_add_friend') or resolve_symbol('api_add_friend')
    if not fn:
        raise RuntimeError('_api_add_friend not found')
    fn.argtypes = [c_int, c_int]
    fn.restype = ctypes.c_bool
    return bool(fn(c_int(a), c_int(b)))

def _api_remove_friend_py(a: int, b: int):
    fn = resolve_symbol('_api_remove_friend') or resolve_symbol('api_remove_friend')
    if not fn:
        raise RuntimeError('_api_remove_friend not found')
    fn.argtypes = [c_int, c_int]
    fn.restype = ctypes.c_bool
    return bool(fn(c_int(a), c_int(b)))

def _api_add_interests_py(uid: int, interests: str):
    fn = resolve_symbol('_api_add_interests') or resolve_symbol('api_add_interests')
    if not fn:
        raise RuntimeError('_api_add_interests not found')
    fn.argtypes = [c_int, c_char_p]
    fn.restype = ctypes.c_bool
    return bool(fn(c_int(uid), c_char_p(interests.encode('utf-8'))))

def _api_get_user_interests_py(uid: int):
    fn = resolve_symbol('_api_get_user_interests') or resolve_symbol('api_get_user_interests')
    if not fn:
        raise RuntimeError('_api_get_user_interests not found')
    return call_str(fn, uid)

def _api_list_all_users_py():
    fn = resolve_symbol('_api_list_all_users') or resolve_symbol('api_list_all_users')
    if not fn:
        raise RuntimeError('_api_list_all_users not found')
    return call_str(fn)

def _api_print_user_info_py(uid: int):
    fn = resolve_symbol('_api_print_user_info') or resolve_symbol('api_print_user_info')
    if not fn:
        raise RuntimeError('_api_print_user_info not found')
    return call_str(fn, uid)

def _api_recommend_mutual_py(uid: int, k: int):
    fn = resolve_symbol('_api_recommend_mutual') or resolve_symbol('api_recommend_mutual')
    if not fn:
        raise RuntimeError('_api_recommend_mutual not found')
    return call_mixed_str_or_int(fn, uid, k)

def _api_recommend_weighted_py(uid: int, k: int):
    fn = resolve_symbol('_api_recommend_weighted') or resolve_symbol('api_recommend_weighted')
    if not fn:
        raise RuntimeError('_api_recommend_weighted not found')
    return call_mixed_str_or_int(fn, uid, k)

def _api_shortest_path_py(a: int, b: int):
    fn = resolve_symbol('_api_shortest_path') or resolve_symbol('api_shortest_path')
    if not fn:
        raise RuntimeError('_api_shortest_path not found')
    return call_str(fn, a, b)

def _api_connected_components_py():
    fn = resolve_symbol('_api_connected_components') or resolve_symbol('api_connected_components')
    if not fn:
        raise RuntimeError('_api_connected_components not found')
    return call_str(fn)

def _api_suggest_prefix_py(prefix: str, k: int):
    fn = resolve_symbol('_api_suggest_prefix') or resolve_symbol('api_suggest_prefix')
    if not fn:
        raise RuntimeError('_api_suggest_prefix not found')
    return call_str(fn, prefix, k)

def _api_save_network_py(path: str):
    fn = resolve_symbol('_api_save_network') or resolve_symbol('api_save_network')
    if not fn:
        raise RuntimeError('_api_save_network not found')
    fn.argtypes = [c_char_p]
    fn.restype = ctypes.c_bool
    return bool(fn(c_char_p(path.encode('utf-8'))))

def _api_load_network_py(path: str):
    fn = resolve_symbol('_api_load_network') or resolve_symbol('api_load_network')
    if not fn:
        raise RuntimeError('_api_load_network not found')
    fn.argtypes = [c_char_p]
    fn.restype = ctypes.c_bool
    return bool(fn(c_char_p(path.encode('utf-8'))))

# ----------------- Flask app -----------------
app = Flask(
    __name__,
    template_folder=_project_path('..', 'templates'),
    static_folder=_project_path('..', 'static')
)

def ok(data):
    return jsonify(success=True, data=data)

def fail(msg, code=500):
    return jsonify(success=False, error=str(msg)), code

def lib_missing():
    return fail('Shared library not found. Build it (make) and ensure libfriend.dylib or libfriend.so is in build/')

@app.route('/')
def index():
    return render_template('index.html')

@app.route('/user')
def user_page():
    return render_template('user.html')

# ---------------- API endpoints ----------------

@app.route('/api/add_user', methods=['POST'])
def api_add_user():
    if lib is None:
        return lib_missing()
    try:
        name = (request.json or {}).get('name', '')
        res = _api_add_user_py(name)
        return ok({'id': res})
    except Exception as e:
        return fail(e)

@app.route('/api/add_user_with_id', methods=['POST'])
def api_add_user_with_id():
    if lib is None:
        return lib_missing()
    try:
        body = request.json or {}
        name = body.get('name', '')
        uid = int(body.get('id', 0))
        res = _api_add_user_with_id_py(name, uid)
        return ok({'id': res})
    except Exception as e:
        return fail(e)

@app.route('/api/remove_user', methods=['POST'])
def api_remove_user():
    if lib is None:
        return lib_missing()
    try:
        uid = int((request.json or {}).get('id', 0))
        res = _api_remove_user_py(uid)
        return ok({'removed': res})
    except Exception as e:
        return fail(e)

@app.route('/api/add_friend', methods=['POST'])
def api_add_friend():
    if lib is None:
        return lib_missing()
    try:
        body = request.json or {}
        a = int(body.get('a', 0)); b = int(body.get('b', 0))
        res = _api_add_friend_py(a, b)
        return ok({'added': res})
    except Exception as e:
        return fail(e)

@app.route('/api/remove_friend', methods=['POST'])
def api_remove_friend():
    if lib is None:
        return lib_missing()
    try:
        body = request.json or {}
        a = int(body.get('a', 0)); b = int(body.get('b', 0))
        res = _api_remove_friend_py(a, b)
        return ok({'removed': res})
    except Exception as e:
        return fail(e)

@app.route('/api/add_interests', methods=['POST'])
def api_add_interests():
    if lib is None:
        return lib_missing()
    try:
        body = request.json or {}
        uid = int(body.get('id', 0))
        interests = body.get('interests', '')
        # allow list or comma-string
        if isinstance(interests, list):
            interests = ",".join(str(x) for x in interests)
        res = _api_add_interests_py(uid, interests)
        return ok({'updated': res})
    except Exception as e:
        return fail(e)

@app.route('/api/get_interests/<int:uid>', methods=['GET'])
def api_get_interests(uid):
    if lib is None:
        return lib_missing()
    try:
        s = _api_get_user_interests_py(uid)
        return ok({'interests': try_parse_json(s)})
    except Exception as e:
        return fail(e)

@app.route('/api/list_users', methods=['GET'])
def api_list_users():
    if lib is None:
        return lib_missing()
    try:
        s = _api_list_all_users_py()
        return ok({'users': try_parse_json(s)})
    except Exception as e:
        return fail(e)

@app.route('/api/recommend_mutual/<int:uid>/<int:k>', methods=['GET'])
def api_recommend_mutual(uid, k):
    if lib is None:
        return lib_missing()
    try:
        s = _api_recommend_mutual_py(uid, k)
        return ok({'recommendations': parse_mutual_text(s)})
    except Exception as e:
        return fail(e)

@app.route('/api/recommend_weighted/<int:uid>/<int:k>', methods=['GET'])
def api_recommend_weighted(uid, k):
    if lib is None:
        return lib_missing()
    try:
        s = _api_recommend_weighted_py(uid, k)
        return ok({'recommendations': parse_weighted_text(s)})
    except Exception as e:
        return fail(e)

@app.route('/api/communities', methods=['GET'])
def api_communities():
    if lib is None:
        return lib_missing()
    try:
        s = _api_connected_components_py()
        return ok({'communities': try_parse_json(s)})
    except Exception as e:
        return fail(e)

@app.route('/api/shortest_path/<int:a>/<int:b>', methods=['GET'])
def api_shortest(a, b):
    if lib is None:
        return lib_missing()
    try:
        s = _api_shortest_path_py(a, b)
        return ok({'path': try_parse_json(s)})
    except Exception as e:
        return fail(e)

@app.route('/api/user/<int:uid>', methods=['GET'])
def api_user(uid):
    if lib is None:
        return lib_missing()
    try:
        s = _api_print_user_info_py(uid)
        return ok({'user': try_parse_json(s)})
    except Exception as e:
        return fail(e)

@app.route('/api/save', methods=['POST'])
def api_save():
    if lib is None:
        return lib_missing()
    try:
        path = (request.json or {}).get('path', 'network.txt')
        res = _api_save_network_py(path)
        return ok({'saved': res, 'path': path})
    except Exception as e:
        return fail(e)

@app.route('/api/load', methods=['POST'])
def api_load():
    if lib is None:
        return lib_missing()
    try:
        path = (request.json or {}).get('path', 'network.txt')
        res = _api_load_network_py(path)
        return ok({'loaded': res, 'path': path})
    except Exception as e:
        return fail(e)

@app.route('/api/suggest/<path:prefix>/<int:k>', methods=['GET'])
def api_suggest(prefix, k):
    if lib is None:
        return lib_missing()
    try:
        s = _api_suggest_prefix_py(prefix, k)
        return ok({'suggestions': try_parse_json(s)})
    except Exception as e:
        return fail(e)

if __name__ == '__main__':
    port = int(os.environ.get('PORT', '5000'))
    print("Starting Flask on port", port)
    # Disable reloader to avoid multiprocessing resource tracker warnings
    app.run(host='127.0.0.1', port=port, debug=True, use_reloader=False)