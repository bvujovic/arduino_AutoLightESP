const TEST = false;

var id = 0;
var cnt = 0;
const maxCnt = 10;
const sepRows = '\n';
const sepData = ';';

//B var myVar = 
setInterval(myTimer, 1000);

function myTimer() {
    
    getStatus();
}

function getStatus() {
    if (TEST) {
        const resp = ++id + `;420;115;0;12`;
        parseStatus(resp);
    }
    else {
        var xhttp = new XMLHttpRequest();
        xhttp.onreadystatechange = function () {
            if (this.readyState == 4 && this.status == 200)
                parseStatus(this.responseText);
        };
        xhttp.open('GET', 'get_status', true); xhttp.send();
    }
}

function parseStatus(resp) {
    var a = resp.split(sepRows);
    for (var s of a) {
        s = s.trim();
        if (s.length === 0) continue;
        addTableRow(s.split(sepData));
    }
}

function addTableRow(status) {
    var table = document.getElementById("tblCurrData");
    if (cnt++ >= maxCnt)
        table.deleteRow(maxCnt);
    var row = table.insertRow(1);
    var cell0 = row.insertCell(0);
    var cell1 = row.insertCell(1);
    var cell2 = row.insertCell(2);
    var cell3 = row.insertCell(3);
    //B var cell4 = row.insertCell(4);
    cell0.innerHTML = id = parseInt(status[0]);
    cell1.innerHTML = status[1];
    cell2.innerHTML = status[2];
    cell3.innerHTML = status[3] == '0' ? 'OFF' : 'ON';
    //B cell4.innerHTML = status[4];
}