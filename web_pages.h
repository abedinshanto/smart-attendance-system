const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>Smart Attendance v3.3</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link href="https://fonts.googleapis.com/css2?family=Inter:wght@400;600&display=swap" rel="stylesheet">
  <style>
    :root { --primary: #2563eb; --danger: #dc2626; --success: #16a34a; --bg: #f3f4f6; --card: #ffffff; }
    body { font-family: 'Inter', sans-serif; margin: 0; padding: 0; background-color: var(--bg); color: #1f2937; }
    
    .navbar { background-color: var(--primary); color: white; padding: 1rem; display: flex; justify-content: space-between; align-items: center; box-shadow: 0 2px 4px rgba(0,0,0,0.1); }
    .nav-brand { font-weight: 600; font-size: 1.25rem; }
    .nav-links button { background: transparent; border: none; color: white; padding: 0.5rem 1rem; cursor: pointer; font-size: 1rem; opacity: 0.8; transition: opacity 0.2s; }
    .nav-links button:hover { opacity: 1; }
    .nav-links button.active { opacity: 1; font-weight: 600; border-bottom: 2px solid white; }
    
    .container { max-width: 1000px; margin: 2rem auto; padding: 0 1rem; }
    .card { background: var(--card); border-radius: 0.5rem; box-shadow: 0 1px 3px rgba(0,0,0,0.1); padding: 1.5rem; margin-bottom: 1.5rem; display: none; border-top: 4px solid var(--primary); }
    .card.active { display: block; animation: fadeIn 0.3s; }
    @keyframes fadeIn { from { opacity: 0; } to { opacity: 1; } }
    
    .stat-grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(200px, 1fr)); gap: 1rem; margin-bottom: 1.5rem; }
    
    /* Pastel Cards */
    .stat-card { padding: 1.5rem; border-radius: 0.5rem; box-shadow: 0 1px 2px rgba(0,0,0,0.05); color: #1f2937; }
    .stat-card.blue { background-color: #dbeafe; border: 1px solid #bfdbfe; }
    .stat-card.yellow { background-color: #fef3c7; border: 1px solid #fde68a; }
    .stat-card.green { background-color: #dcfce7; border: 1px solid #bbf7d0; }
    
    .stat-label { font-size: 0.875rem; font-weight: 600; opacity: 0.8; margin-bottom: 0.25rem; }
    .stat-value { font-size: 2rem; font-weight: 700; }
    .stat-sub { font-size: 0.875rem; opacity: 0.8; }
    
    table { width: 100%; border-collapse: collapse; margin-top: 1rem; }
    th, td { padding: 0.75rem; text-align: left; border-bottom: 1px solid #e5e7eb; }
    th { background-color: #f9fafb; font-weight: 600; font-size: 0.875rem; color: #374151; }
    tr:hover { background-color: #f9fafb; }
    
    .btn { padding: 0.5rem 1rem; border-radius: 0.375rem; border: none; cursor: pointer; font-weight: 500; transition: background 0.2s; }
    .btn-primary { background-color: var(--primary); color: white; }
    .btn-primary:hover { background-color: #1d4ed8; }
    .btn-danger { background-color: var(--danger); color: white; }
    .btn-danger:hover { background-color: #b91c1c; }
    .btn-sm { padding: 0.25rem 0.5rem; font-size: 0.75rem; width: auto; }
    
    input, select { width: 100%; padding: 0.5rem; border: 1px solid #d1d5db; border-radius: 0.375rem; box-sizing: border-box; margin-bottom: 0.5rem; }
    input:focus { outline: 2px solid var(--primary); border-color: transparent; }
    
    .badge { padding: 0.25rem 0.5rem; border-radius: 9999px; font-size: 0.75rem; font-weight: 500; }
    .badge-Present { background-color: #dcfce7; color: #166534; }
    .badge-Late { background-color: #fef9c3; color: #854d0e; }
    .badge-Denied { background-color: #fee2e2; color: #991b1b; }
    .badge-Exit { background-color: #e0f2fe; color: #075985; }
    
    /* Accordion for Routines */
    .accordion { border: 1px solid #e5e7eb; border-radius: 0.5rem; overflow: hidden; margin-bottom: 0.5rem; background: white; }
    .accordion-header { background: #f3f4f6; padding: 1rem; cursor: pointer; font-weight: 600; display: flex; justify-content: space-between; color: #374151; }
    .accordion-header:hover { background: #e5e7eb; }
    .accordion-content { padding: 1rem; display: none; border-top: 1px solid #e5e7eb; }
    .accordion.open .accordion-content { display: block; }
  </style>
</head>
<body>
<div class="navbar">
  <div class="nav-brand">Smart Attendance</div>
  <div class="nav-links">
    <button class="active" onclick="showTab('dashboard')">Dashboard</button>
    <button onclick="showTab('students')">Students</button>
    <button onclick="showTab('routines')">Routines</button>
    <button onclick="showTab('reports')">Reports</button>
  </div>
</div>
<div class="container">
  <!-- DASHBOARD -->
  <div id="dashboard" class="card active">
    <!-- Pastel Stat Cards -->
    <div class="stat-grid">
      <div class="stat-card blue">
        <div class="stat-label">System Time</div>
        <div class="stat-value" id="sysTime">--:--</div>
      </div>
      <div class="stat-card yellow">
        <div class="stat-label">Last Scanned</div>
        <div class="stat-value" id="lastScanName" style="font-size:1.5rem">Waiting...</div>
        <div class="stat-sub" id="lastScanTime">--:--</div>
      </div>
      <div class="stat-card green">
        <div class="stat-label">Total Present</div>
        <div class="stat-value" id="totalPresent">0</div>
        <div class="stat-sub">Today</div>
      </div>
    </div>
    
    <div style="display:flex; justify-content:space-between; align-items:center; margin-bottom:1rem;">
      <h2 style="color: #374151;">Activity Log</h2>
      <input type="date" id="dashDate" style="width:auto; padding:0.5rem;" onchange="loadLiveLogs()">
    </div>
    
    <table id="liveTable">
      <thead><tr><th>Time</th><th>Name</th><th>Dept</th><th>Sem</th><th>Phone</th><th>Status</th></tr></thead>
      <tbody></tbody>
    </table>
  </div>
  <!-- STUDENTS -->
  <div id="students" class="card">
    <div style="display:flex; justify-content:space-between; align-items:center; margin-bottom:1rem;">
      <h2 style="color: #374151;">Students</h2>
      <button class="btn btn-primary" onclick="openStudentForm()">+ Add Student</button>
    </div>
    
    <!-- Form -->
    <div id="studentForm" style="background: #f9fafb; padding: 1.5rem; border-radius: 0.5rem; margin-bottom: 1.5rem; display:none; border: 1px solid #e5e7eb;">
      <h3 style="margin-top:0;">Student Details</h3>
      <div style="display:grid; grid-template-columns: 1fr 1fr; gap:1rem;">
        <div><label>Name</label><input type="text" id="s_name"></div>
        <div><label>Mobile</label><input type="text" id="s_contact"></div>
        <div><label>Dept</label><input type="text" id="s_dept" placeholder="CSE"></div>
        <div><label>Semester</label><input type="text" id="s_sem" placeholder="3rd"></div>
        <div><label>Roll</label><input type="text" id="s_roll"></div>
        <div><label>Blood Group</label><input type="text" id="s_blood"></div>
        <div><label>Reg No</label><input type="text" id="s_reg"></div>
        <div><label>Card UID (Read-Only)</label><input type="text" id="s_uid" readonly style="background:#e5e7eb; cursor:not-allowed;"></div>
      </div>
      <div style="margin-top:1rem; text-align:right;">
        <button class="btn btn-danger" onclick="document.getElementById('studentForm').style.display='none'">Cancel</button>
        <button class="btn btn-primary" onclick="saveStudent()">Save</button>
      </div>
    </div>
    <input type="text" id="studentSearch" placeholder="Search by Name, Dept or Roll..." onkeyup="filterStudents()" style="padding: 0.75rem;">
    <table id="studentTable">
      <thead><tr><th>Name</th><th>Dept</th><th>Sem</th><th>Roll</th><th>Mobile</th><th>Blood</th><th>Actions</th></tr></thead>
      <tbody></tbody>
    </table>
  </div>
  <!-- ROUTINES -->
  <div id="routines" class="card">
    <div style="display:flex; justify-content:space-between; align-items:center; margin-bottom:1rem;">
      <h2 style="color: #374151;">Class Routines</h2>
      <button class="btn btn-primary" onclick="openRoutineForm()">+ New Routine</button>
    </div>
    
    <!-- Routine Editor -->
    <div id="routineForm" style="background: #f9fafb; padding: 1.5rem; border-radius: 0.5rem; margin-bottom: 1.5rem; display:none; border: 1px solid #e5e7eb;">
      <h3 style="margin-top:0;">Edit Routine</h3>
      <div style="display:grid; grid-template-columns: 1fr 1fr; gap:1rem;">
        <div><label>Dept</label><input type="text" id="r_dept"></div>
        <div><label>Sem</label><input type="text" id="r_sem"></div>
        <div><label>Start</label><input type="time" id="r_start"></div>
        <div><label>End</label><input type="time" id="r_end"></div>
      </div>
      <label style="margin-top:1rem;">Breaks</label>
      <div id="breaksContainer"></div>
      <button class="btn btn-sm btn-primary" onclick="addBreakField()" style="margin-top:0.5rem;">+ Add Break</button>
      <div style="margin-top:1rem; text-align:right;">
         <button class="btn btn-danger" onclick="document.getElementById('routineForm').style.display='none'">Cancel</button>
         <button class="btn btn-primary" onclick="saveRoutine()">Save</button>
      </div>
    </div>
    <div id="routineList"></div>
  </div>
  <!-- REPORTS -->
  <div id="reports" class="card">
    <h2 style="color: #374151;">Reports</h2>
    <p>Download daily attendance reports.</p>
    <div style="display:flex; gap:1rem; align-items:center;">
      <input type="date" id="reportDate" style="width:auto; padding: 0.75rem;">
      <button class="btn btn-primary" onclick="downloadReport()">Download CSV</button>
    </div>
    
    <hr style="margin: 2rem 0; border:0; border-top:1px solid #eee;">
    
    <h3 style="color: #dc2626;">Danger Zone</h3>
    <p>This action is irreversible. It will delete all attendance history.</p>
    <button class="btn btn-danger" onclick="clearLogs()">Clear All Logs</button>
  </div>
</div>
<script>
  let allStudents = [];
  let allRoutines = {};
  
  // Init
  document.getElementById('dashDate').valueAsDate = new Date();
  document.getElementById('reportDate').valueAsDate = new Date();
  
  setInterval(updateStatus, 1000);
  loadStudents();
  loadRoutines();
  loadLiveLogs();
  
  function showTab(id) {
    document.querySelectorAll('.card').forEach(c => c.classList.remove('active'));
    document.getElementById(id).classList.add('active');
    document.querySelectorAll('.nav-links button').forEach(b => b.classList.remove('active'));
    event.target.classList.add('active');
  }
  // --- DASHBOARD ---
  function updateStatus() {
    fetch('/api/status').then(r => r.json()).then(data => {
      document.getElementById('sysTime').innerText = data.time;
      document.getElementById('totalPresent').innerText = data.totalPresent;
      
      if(data.lastUid) {
        document.getElementById('lastScanName').innerText = data.lastName || "Unknown Card";
        document.getElementById('lastScanTime').innerText = data.lastTime;
        
        // Auto-fill UID for new student
        if(document.getElementById('studentForm').style.display !== 'none') {
           if(document.getElementById('s_uid').value !== data.lastUid) {
             const existing = allStudents.find(s => s.uid === data.lastUid);
             if(existing) editStudent(existing.uid);
             else document.getElementById('s_uid').value = data.lastUid;
           }
        }
      }
    });
  }
  function loadLiveLogs() {
    const dateInput = document.getElementById('dashDate');
    let dateStr = dateInput.value;
    
    if(!dateStr) {
       // Manual Local Date String construction to avoid UTC issues
       const now = new Date();
       const yyyy = now.getFullYear();
       const mm = String(now.getMonth() + 1).padStart(2, '0');
       const dd = String(now.getDate()).padStart(2, '0');
       dateStr = `${yyyy}-${mm}-${dd}`;
       dateInput.value = dateStr;
    }
    
    // Add timestamp to prevent caching
    fetch('/api/logs?date=' + dateStr + '&t=' + Date.now()).then(r => r.json()).then(data => {
      const tbody = document.querySelector('#liveTable tbody');
      if(!data || data.length === 0) {
        tbody.innerHTML = '<tr><td colspan="6" style="text-align:center; color:#666;">No logs found for ' + dateStr + '</td></tr>';
      } else {
        data.reverse(); 
        tbody.innerHTML = data.map(l => {
          // Format Time to 12h AM/PM
          let timeDisplay = l.time;
          if(l.time && l.time.includes(':')) {
            const [h, m] = l.time.split(':');
            const hour = parseInt(h);
            const suffix = hour >= 12 ? 'PM' : 'AM';
            const hour12 = hour % 12 || 12;
            timeDisplay = `${hour12}:${m} ${suffix}`;
          }
           // Use zero-width space if contact is missing to keep cell structure
          const contact = l.contact && l.contact !== 'null' ? l.contact : '-';
          return `
          <tr>
            <td>${timeDisplay}</td>
            <td>${l.name}</td>
            <td>${l.dept}</td>
            <td>${l.sem}</td>
            <td>${contact}</td>
            <td><span class="badge badge-${l.status.split(' ')[0]}">${l.status}</span></td>
          </tr>
        `}).join('');
      }
    }).catch(e => {
       console.error(e);
       document.querySelector('#liveTable tbody').innerHTML = '<tr><td colspan="6" style="text-align:center; color:red;">Error loading logs</td></tr>';
    });
  }
  // --- STUDENTS ---
  function loadStudents() {
    fetch('/api/students').then(r => r.json()).then(data => {
      allStudents = data;
      renderStudents(data);
    });
  }
  function renderStudents(list) {
    const tbody = document.querySelector('#studentTable tbody');
    tbody.innerHTML = list.map(s => `
      <tr>
        <td>${s.name}</td>
        <td>${s.dept}</td>
        <td>${s.sem}</td>
        <td>${s.roll}</td>
        <td>${s.contact}</td>
        <td>${s.blood}</td>
        <td>
          <button class="btn btn-sm btn-primary" onclick="editStudent('${s.uid}')">Edit</button>
          <button class="btn btn-sm btn-danger" onclick="deleteStudent('${s.uid}')">Del</button>
        </td>
      </tr>
    `).join('');
  }
  function filterStudents() {
    const q = document.getElementById('studentSearch').value.toLowerCase();
    const filtered = allStudents.filter(s => 
      s.name.toLowerCase().includes(q) || s.dept.toLowerCase().includes(q) || s.roll.toLowerCase().includes(q)
    );
    renderStudents(filtered);
  }
  function openStudentForm() {
    document.getElementById('studentForm').style.display = 'block';
    ['s_uid','s_name','s_dept','s_sem','s_roll','s_reg','s_contact','s_blood'].forEach(id => document.getElementById(id).value = '');
    alert("Scan a card now to auto-fill UID");
  }
  function editStudent(uid) {
    const s = allStudents.find(x => x.uid === uid);
    if(!s) return;
    document.getElementById('studentForm').style.display = 'block';
    document.getElementById('s_uid').value = s.uid;
    document.getElementById('s_name').value = s.name;
    document.getElementById('s_dept').value = s.dept;
    document.getElementById('s_sem').value = s.sem;
    document.getElementById('s_roll').value = s.roll;
    document.getElementById('s_reg').value = s.reg;
    document.getElementById('s_contact').value = s.contact;
    document.getElementById('s_blood').value = s.blood;
  }
  function saveStudent() {
    const data = new URLSearchParams();
    ['uid','name','dept','sem','roll','reg','contact','blood'].forEach(key => {
      data.append(key, document.getElementById('s_'+key).value);
    });
    fetch('/api/student', { method: 'POST', body: data }).then(() => {
      loadStudents();
      document.getElementById('studentForm').style.display = 'none';
      alert('Saved!');
    });
  }
  
  function deleteStudent(uid) {
    if(confirm('Delete student?')) {
      fetch('/api/student?uid=' + uid, { method: 'DELETE' }).then(() => loadStudents());
    }
  }
  // --- ROUTINES ---
  function loadRoutines() {
    fetch('/api/routines').then(r => {
        if(!r.ok) throw new Error("HTTP " + r.status);
        return r.json();
    }).then(data => {
      console.log("Routines Data:", data);
      allRoutines = data;
      const container = document.getElementById('routineList');
      container.innerHTML = '';
      
      if(Object.keys(data).length === 0) {
        container.innerHTML = '<p style="color:#666;">No routines configured.</p>';
        return;
      }
      
      for(const dept in data) {
        const deptDiv = document.createElement('div');
        deptDiv.className = 'accordion';
        deptDiv.innerHTML = `<div class="accordion-header" onclick="this.parentElement.classList.toggle('open')">${dept} <span>▼</span></div>`;
        
        const content = document.createElement('div');
        content.className = 'accordion-content';
        content.style.backgroundColor = '#f9fafb';
        content.style.padding = '1rem';
        
        // CSS Grid for Semester Cards
        const cardGrid = document.createElement('div');
        cardGrid.style.display = 'grid';
        cardGrid.style.gridTemplateColumns = 'repeat(auto-fill, minmax(200px, 1fr))';
        cardGrid.style.gap = '1rem';
        
        for(const sem in data[dept]) {
          const r = data[dept][sem];
          cardGrid.innerHTML += `
            <div style="background:white; padding:1rem; border-radius:0.5rem; border:1px solid #e5e7eb; box-shadow:0 1px 2px rgba(0,0,0,0.05);">
              <div style="display:flex; justify-content:space-between; align-items:center; margin-bottom:0.5rem;">
                <strong style="font-size:1.1rem; color:#374151;">${sem}</strong>
                <button class="btn btn-sm btn-primary" onclick="editRoutine('${dept}','${sem}')">Edit</button>
              </div>
              <div style="font-size:0.9rem; color:#6b7280; margin-bottom:0.5rem;">
                ⏱ ${r.start} - ${r.end}
              </div>
              <div style="font-size:0.85rem; color:#6b7280;">
                 <span style="background:#e5e7eb; padding:2px 6px; border-radius:4px;">${r.breaks.length} Breaks</span>
              </div>
            </div>
          `;
        }
        content.appendChild(cardGrid);
        deptDiv.appendChild(content);
        container.appendChild(deptDiv);
      }
    }).catch(e => {
       console.error("Routine Load Error:", e);
       document.getElementById('routineList').innerHTML = '<p style="color:red; text-align:center;">Error loading routines. <br>Please Check Serial Monitor.</p>';
    });
  }
  function openRoutineForm() {
    document.getElementById('routineForm').style.display = 'block';
    document.getElementById('r_dept').value = '';
    document.getElementById('r_sem').value = '';
    document.getElementById('breaksContainer').innerHTML = '';
  }
  function addBreakField(s='', e='') {
    const div = document.createElement('div');
    div.className = 'break-item';
    div.style.marginBottom = '0.5rem';
    div.innerHTML = `<input type="time" class="b-s" value="${s}" style="width:40%"> to <input type="time" class="b-e" value="${e}" style="width:40%"> <button class="btn btn-sm btn-danger" onclick="this.parentElement.remove()">X</button>`;
    document.getElementById('breaksContainer').appendChild(div);
  }
  function editRoutine(dept, sem) {
    const r = allRoutines[dept][sem];
    document.getElementById('routineForm').style.display = 'block';
    document.getElementById('r_dept').value = dept;
    document.getElementById('r_sem').value = sem;
    document.getElementById('r_start').value = r.start;
    document.getElementById('r_end').value = r.end;
    document.getElementById('breaksContainer').innerHTML = '';
    r.breaks.forEach(b => addBreakField(b.s, b.e));
  }
  function saveRoutine() {
    const breaks = [];
    document.querySelectorAll('.break-item').forEach(d => {
      breaks.push({ s: d.querySelector('.b-s').value, e: d.querySelector('.b-e').value });
    });
    
    const payload = {
      start: document.getElementById('r_start').value,
      end: document.getElementById('r_end').value,
      breaks: breaks
    };
    
    const data = new URLSearchParams();
    data.append('dept', document.getElementById('r_dept').value);
    data.append('sem', document.getElementById('r_sem').value);
    data.append('json', JSON.stringify(payload));
    
    fetch('/api/routine', { method: 'POST', body: data }).then(() => {
      loadRoutines();
      document.getElementById('routineForm').style.display = 'none';
      alert('Saved!');
    });
  }
  // --- REPORTS ---
  function downloadReport() {
    const date = document.getElementById('reportDate').value;
    if(!date) return alert('Select a date');
    
    // Create a hidden link to force download
    const link = document.createElement('a');
    link.href = `/api/download_logs?date=${date}`;
    link.download = `report_${date}.csv`;
    document.body.appendChild(link);
    link.click();
    document.body.removeChild(link);
  }
  
  function clearLogs() {
    const date = document.getElementById('reportDate').value;
    if(!date) return alert('Select a date to delete logs for.');
    
    if(confirm("Delete logs for " + date + "? This cannot be undone.")) {
      fetch('/api/clear_logs?date=' + date, { method: 'POST' }).then(r => r.text()).then(msg => {
         alert(msg);
         if(date === document.getElementById('dashDate').value) {
            loadLiveLogs(); // Refresh dashboard if same date
            updateStatus();
         }
      });
    }
  }
</script>
</body>
</html>
)rawliteral";