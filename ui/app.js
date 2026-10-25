// LCS UI shell — talks to the C++ REST API on the same origin.
(() => {
  const $ = (id) => document.getElementById(id);
  const log = $("log");
  const append = (line) => {
    log.textContent = `[${new Date().toLocaleTimeString()}] ${line}\n` + log.textContent;
  };

  async function api(path, opts) {
    const res = await fetch(path, opts);
    if (!res.ok) throw new Error(`${path}: HTTP ${res.status}`);
    return res.json();
  }

  function render(s) {
    $("state").textContent = s.state;
    $("pos").textContent = `(${s.position.x}, ${s.position.y}, ${s.position.z})`;
    $("env").textContent = `${s.temperatureK} K · ${s.pressurePa} Pa`;
  }

  async function refresh() {
    try {
      render(await api("/api/state"));
      $("conn").textContent = "connected";
      $("conn").style.color = "var(--good)";
    } catch (e) {
      $("conn").textContent = "offline";
      $("conn").style.color = "var(--danger)";
    }
  }

  async function command(name) {
    try {
      const s = await api(`/api/${name}`, { method: "POST" });
      render(s);
      append(`command ${name} ok → ${s.state}`);
    } catch (e) {
      append(`command ${name} failed: ${e.message}`);
    }
  }

  document.querySelectorAll("[data-cmd]").forEach((b) =>
    b.addEventListener("click", () => command(b.dataset.cmd))
  );

  $("moveBtn").addEventListener("click", async () => {
    const body = JSON.stringify({ x: +$("mx").value, y: +$("my").value, z: +$("mz").value });
    try {
      const s = await api("/api/move", { method: "POST", body });
      render(s);
      append(`move → (${s.position.x}, ${s.position.y}, ${s.position.z})`);
    } catch (e) {
      append(`move failed: ${e.message}`);
    }
  });

  $("magBtn").addEventListener("click", async () => {
    const q = `x=${+$("gx").value || 0}&y=${+$("gy").value || 0}&z=${+$("gz").value || 0}`;
    try {
      const r = await api(`/api/math/mag?${q}`);
      $("magOut").textContent = r.magnitude;
      append(`magnitude(${q}) = ${r.magnitude}`);
    } catch (e) {
      append(`magnitude failed: ${e.message}`);
    }
  });

  // ---- control planes ----
  async function refreshPlanes() {
    try {
      const [safety, optics, alarms, audit] = await Promise.all([
        api("/api/safety"), api("/api/optics"), api("/api/alarms"), api("/api/audit"),
      ]);
      $("safety").textContent = safety.allSatisfied ? "all interlocks satisfied" : "INTERLOCKS OPEN";
      $("safety").style.color = safety.allSatisfied ? "var(--good)" : "var(--danger)";
      $("safetyFailures").textContent = safety.failures.join(", ");
      $("optics").textContent = `λ=${optics.wavelengthNm}nm · NA=${optics.numericalAperture} · ${optics.resolutionNm}nm`;
      $("alarms").textContent = alarms.active.length
        ? alarms.active.map((a) => `[${a.severity}] ${a.code}: ${a.message}`).join("\n")
        : "no active alarms";
      $("alarms").style.color = alarms.hasCritical ? "var(--danger)" : "var(--text)";
      $("audit").textContent = audit.slice(-12).reverse()
        .map((e) => `#${e.id} ${e.plane}.${e.action}: ${e.detail}`).join("\n");
    } catch { /* server offline; refresh() reports it */ }
  }

  async function post(path, body) {
    const res = await fetch(path, { method: "POST", headers: { "Content-Type": "application/json" }, body: JSON.stringify(body) });
    const data = await res.json().catch(() => ({}));
    if (!res.ok) throw new Error(data.error || `HTTP ${res.status}`);
    return data;
  }

  $("opticsBtn").addEventListener("click", async () => {
    try {
      await post("/api/optics/set", { wavelengthNm: +$("oWl").value, numericalAperture: +$("oNa").value, reduction: +$("oRed").value });
      append("optics parameters applied");
    } catch (e) { append(`optics rejected: ${e.message}`); }
  });

  $("raiseBtn").addEventListener("click", async () => {
    try { await post("/api/alarms/raise", { code: $("aCode").value, severity: $("aSev").value, message: $("aMsg").value }); append("alarm raised"); }
    catch (e) { append(`alarm raise failed: ${e.message}`); }
  });
  $("clearBtn").addEventListener("click", async () => {
    try { await post("/api/alarms/clear", { code: $("aCode").value }); append("alarm cleared"); }
    catch (e) { append(`alarm clear failed: ${e.message}`); }
  });

  $("pushBtn").addEventListener("click", async () => {
    try { const r = await post("/api/telemetry/push", { sensorId: +$("tSensor").value, value: +$("tValue").value, unit: $("tUnit").value }); append(`telemetry pushed (${r.samples} buffered)`); }
    catch (e) { append(`telemetry push failed: ${e.message}`); }
  });
  $("drainBtn").addEventListener("click", async () => {
    try { const r = await api("/api/telemetry/drain"); append(`telemetry drained (${r.length} samples)`); }
    catch (e) { append(`telemetry drain failed: ${e.message}`); }
  });

  async function job(action) {
    const id = $("jId").value;
    try {
      if (action === "create") await post("/api/jobs/create", { id });
      else await post(`/api/jobs/${id}/${action}`, {});
      append(`job ${id} ${action} ok`);
    } catch (e) { append(`job ${id} ${action} failed: ${e.message}`); }
  }
  ["create", "validate", "start", "complete"].forEach((a) =>
    $(`j${a[0].toUpperCase()}${a.slice(1)}`).addEventListener("click", () => job(a))
  );

  refresh();
  refreshPlanes();
  setInterval(refresh, 1000);
  setInterval(refreshPlanes, 2000);
  append("UI shell started");
})();
