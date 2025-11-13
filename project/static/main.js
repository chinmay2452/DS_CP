// main.js - UI layer using fetch to talk to Flask endpoints

async function apiFetch(path, method="GET", body=null) {
  const opts = { method, headers: {} };
  if (body !== null) {
    opts.headers["Content-Type"] = "application/json";
    opts.body = JSON.stringify(body);
  }
  try {
    const res = await fetch(path, opts);
    if (!res.ok) {
      const txt = await res.text().catch(()=>null);
      throw new Error(`HTTP ${res.status} ${res.statusText} ${txt||''}`);
    }
    const j = await res.json().catch(()=>null);
    return j;
  } catch (e) {
    // show global raw area if present
    const raw = document.getElementById("raw_response");
    if (raw) raw.textContent = `Server not reachable\n${e}`;
    throw e;
  }
}

function pretty(o) {
  try { return JSON.stringify(o, null, 2); } catch(e) { return String(o); }
}

function showOut(id, obj) {
  const el = document.getElementById(id);
  if (!el) return;
  el.textContent = pretty(obj);
  // also write to raw-response
  const raw = document.getElementById("raw_response");
  if (raw) raw.textContent = `${id} =>\n${pretty(obj)}`;
}

// ---------------- functions (map to endpoints) ----------------

async function addUser() {
  const name = document.getElementById("add_user_name").value.trim();
  if (!name) return alert("Enter name");
  showOut("add_user_result", "Loading...");
  const r = await apiFetch("/api/add_user", "POST", { name });
  showOut("add_user_result", r);
  listUsers();
}

async function addUserWithId() {
  const name = document.getElementById("add_with_id_name").value.trim();
  const id = document.getElementById("add_with_id_id").value.trim();
  if (!name || !id) return alert("Enter name and id");
  showOut("add_with_id_result", "Loading...");
  const r = await apiFetch("/api/add_user_with_id", "POST", { id: parseInt(id), name });
  showOut("add_with_id_result", r);
  listUsers();
}

async function removeUser() {
  const id = document.getElementById("remove_user_id").value.trim();
  if (!id) return alert("Enter id");
  showOut("remove_user_result", "Loading...");
  const r = await apiFetch("/api/remove_user", "POST", { id: parseInt(id) });
  showOut("remove_user_result", r);
  listUsers();
}

async function addFriend() {
  const a = document.getElementById("add_friend_a").value.trim();
  const b = document.getElementById("add_friend_b").value.trim();
  if (!a || !b) return alert("Enter both ids");
  showOut("add_friend_result", "Loading...");
  const r = await apiFetch("/api/add_friend", "POST", { a: parseInt(a), b: parseInt(b) });
  showOut("add_friend_result", r);
}

async function removeFriend() {
  const a = document.getElementById("remove_friend_a").value.trim();
  const b = document.getElementById("remove_friend_b").value.trim();
  if (!a || !b) return alert("Enter both ids");
  showOut("remove_friend_result", "Loading...");
  const r = await apiFetch("/api/remove_friend", "POST", { a: parseInt(a), b: parseInt(b) });
  showOut("remove_friend_result", r);
}

async function addInterests() {
  const id = document.getElementById("add_interests_id").value.trim();
  let list = document.getElementById("add_interests_list").value.trim();
  if (!id || !list) return alert("Enter id and interests");
  showOut("add_interests_result", "Loading...");
  // pass a comma string
  const r = await apiFetch("/api/add_interests", "POST", { id: parseInt(id), interests: list });
  showOut("add_interests_result", r);
}

async function getInterests() {
  const id = document.getElementById("get_interests_id").value.trim();
  if (!id) return alert("Enter id");
  showOut("get_interests_result", "Loading...");
  const r = await apiFetch(`/api/get_interests/${parseInt(id)}`);
  showOut("get_interests_result", r);
}

async function recommendMutual() {
  const id = document.getElementById("rec_mutual_id").value.trim();
  const k = document.getElementById("rec_mutual_k").value.trim() || 5;
  if (!id) return alert("Enter id");
  showOut("rec_mutual_result", "Loading...");
  const r = await apiFetch(`/api/recommend_mutual/${parseInt(id)}/${parseInt(k)}`);
  showOut("rec_mutual_result", r);
}

async function recommendWeighted() {
  const id = document.getElementById("rec_weight_id").value.trim();
  const k = document.getElementById("rec_weight_k").value.trim() || 5;
  if (!id) return alert("Enter id");
  showOut("rec_weight_result", "Loading...");
  const r = await apiFetch(`/api/recommend_weighted/${parseInt(id)}/${parseInt(k)}`);
  showOut("rec_weight_result", r);
}

async function communities() {
  showOut("communities_result", "Loading...");
  const r = await apiFetch("/api/communities");
  showOut("communities_result", r);
}

async function listUsers() {
  showOut("list_users_result", "Loading...");
  const r = await apiFetch("/api/list_users");
  showOut("list_users_result", r);
}

async function shortestPath() {
  const a = document.getElementById("spath_a").value.trim();
  const b = document.getElementById("spath_b").value.trim();
  if (!a || !b) return alert("Enter both");
  showOut("spath_result", "Loading...");
  const r = await apiFetch(`/api/shortest_path/${parseInt(a)}/${parseInt(b)}`);
  showOut("spath_result", r);
}

async function saveNetwork() {
  const filename = document.getElementById("save_filename").value.trim() || "network.dat";
  showOut("save_result", "Loading...");
  const r = await apiFetch("/api/save", "POST", { path: filename });
  showOut("save_result", r);
}

async function loadNetwork() {
  const filename = document.getElementById("load_filename").value.trim() || "network.dat";
  showOut("load_result", "Loading...");
  const r = await apiFetch("/api/load", "POST", { path: filename });
  showOut("load_result", r);
}

async function suggest() {
  const prefix = document.getElementById("suggest_prefix").value.trim() || "";
  const k = document.getElementById("suggest_k").value.trim() || 5;
  showOut("suggest_result", "Loading...");
  const r = await apiFetch(`/api/suggest/${encodeURIComponent(prefix)}/${parseInt(k)}`);
  showOut("suggest_result", r);
}

function viewUser() {
  const id = document.getElementById("view_user_id").value.trim();
  if (!id) return alert("Enter id");
  // open the simple profile route (server has /user/<id>)
  window.location = `/user?id=${encodeURIComponent(id)}`;
}

function openProfile() {
  const id = document.getElementById("profile_id_input").value.trim();
  if (!id) return alert("Enter id");
  window.location = `/user?id=${encodeURIComponent(id)}`;
}

async function exportDot() {
  const fname = document.getElementById("dot_filename").value.trim() || "graph.dot";
  showOut("dot_result", "Not implemented in frontend — use backend export if available.");
  alert("DOT export requires backend endpoint that writes a file. Use Save/Load or implement an export endpoint.");
}

// auto-refresh users on load
document.addEventListener('DOMContentLoaded', () => {
  listUsers();
});