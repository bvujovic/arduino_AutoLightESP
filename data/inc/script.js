function el(id) { return document.getElementById(id); }

function setCurrentButtonColor(curBtn) {
    if (!curBtn.id.startsWith('ch'))
        return;
    const btns = document.getElementById('main').getElementsByTagName('button');
    for (const btn of btns)
        btn.style.backgroundColor = '#000';
    curBtn.style.backgroundColor = '#222';
    selChId = curBtn.id;
}

var setClickAll = true;
function setClickHandler() {
    const butts = document.getElementsByTagName('button');
    for (var i = 0; i < butts.length; i++)
        // if (setClickAll || !butts[i].id.startsWith('mode'))
        if (setClickAll || (!butts[i].id.startsWith('mode') && butts[i].parentNode.tagName != 'HEADER' && butts[i].parentNode.tagName != 'FOOTER'))
            butts[i].addEventListener('click', function () { setCurrentButtonColor(this); send(this.id); }, false);
    setClickAll = false;
}

function send(cmd) {
    if (cmd == 'onOff' && !confirm('Are you sure?'))
        return;
    if (cmd.endsWith('_sel'))
        if (loadSelectedChannels) {
            const mainDiv = document.getElementById('main');
            mainDiv.removeChild(document.getElementById(cmd).parentNode);
        }
        else
            document.getElementById(cmd).disabled = true;
    if (cmd == 'upCh' || cmd == 'downCh') {
        const d = (cmd == 'upCh') ? +2 : -2;
        var btns = document.getElementById('main').getElementsByTagName('button');
        var cntInvisible = 0;
        for (const btn of btns)
            if (btn.parentNode.style.display == 'none')
                cntInvisible++;
        if (cntInvisible == btns.length)
            return;
        for (var i = 0; i < btns.length; i++) {
            if (btns[i].id == selChId) {
                do {
                    var i = i + d;
                    if (i >= btns.length) i = 0;
                    if (i < 0) i = btns.length - 2;
                } while (btns[i].parentNode.style.display == 'none');
                setCurrentButtonColor(btns[i]);
                cmd = btns[i].id;
                break;
            }
        }
    }
    url = 'act?cmd=' + cmd;
    var x = new XMLHttpRequest();
    x.open('GET', url, true);
    x.send(null);
}

const sepRows = '\n';
const sepProps = '|';
const sepChans = ',';

function Tag(id, name, channels) {
    this.id = id;
    this.name = name;
    this.channels = channels;
}
Tag.prototype.chanAdd = function (id) {
    this.channels.push(id);
}
Tag.prototype.chanDel = function (id) {
    const idx = this.channels.indexOf(id);
    if (idx >= 0)
        this.channels.splice(idx, 1);
}
Tag.prototype.toString = function () {
    return this.id + sepProps + this.name + sepProps + this.channels.join(sepChans);
}

var tags = [];

function getTags() {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200) {
            var a = this.responseText.split(sepRows);
            for (const t of a) {
                if (t.trim().length === 0) continue;
                const props = t.split(sepProps);
                var chans = props[2].trim().length > 0 ? props[2].trim().split(sepChans) : [];
                tags.push(new Tag(props[0], props[1], chans));
            }
            displayTagsInPopup();
        }
    };
    xhttp.open('GET', 'getTags', true); xhttp.send();
}

function TagButtonClick(tag) {
    loadSelectedChannels = false;
    displayChannels(); //B getChannels();
    el('search').value = tag;
    chSearch(tag);
}

function displayTags() {
    var s = '';
    for (const t of tags)
        s += '<div> <button id="tag' + t.id + '" onclick="TagButtonClick(\'' + t.name + '\')"; >' + t.name + '</button> ' +
            '<button id="tag' + t.id + '_del">Del</button> </div>';
    el('main').innerHTML = s;
}
function displayTagsInPopup() {
    var s = '';
    for (const t of tags)
        s += "<input type='checkbox' id='tag" + t.id + "' /> <label for='tag" + t.id + "'>" + t.name + "</label> </br>";
    el('frmTagsMain').innerHTML = s;
}
function TagsForChan(chNum) {
    chNum = '' + chNum;
    var s = '';
    for (const t of tags)
        if (typeof t.channels !== 'undefined' && t.channels.indexOf(chNum) >= 0)
            s += t.name + sepChans;
    return s;
}
function TagIDsForChan(chNum) {
    chNum = '' + chNum;
    var a = [];
    for (const t of tags)
        if (t.channels.indexOf(chNum) >= 0)
            a.push('tag' + t.id);
    return a;
}

function Channel(id, name, number, isSelected) {
    this.id = id;
    this.name = name;
    this.number = number;
    this.isSelected = isSelected;
}
Channel.prototype.toString = function () {
    return this.id + sepProps + this.name + sepProps + this.number + sepProps + this.isSelected;
}

var loadSelectedChannels = false;

function getChannels() {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200) {
            var a = this.responseText.split(sepRows);
            var i = 0;
            chans = [];
            for (const c of a) {
                if (c.trim().length === 0) continue;
                const props = c.split(sepProps);
                chans.push(new Channel(i++, props[0], parseInt(props[1]), parseInt(props[3])));
            }
            displayChannels();
        }
    }
    xhttp.open('GET', 'getChannels', true); xhttp.send();
}
var chans = [];
var selChId = 'ch0';

function clearVal(el) { el.value = ''; }

function displayChannels() {
    var s = "<input type='text' id='search' autocomplete='off' onkeyup='chSearch(this.value);' onclick='clearVal(this); chSearch(this.value)' placeholder='Search' />";
    for (i = 0; i < chans.length; i++) {
        const ch = chans[i];
        if (loadSelectedChannels && !ch.isSelected)
            continue;
        tagNames = TagsForChan(ch.number);
        const sel = loadSelectedChannels ? '<<' : '>>';
        const selDisabled = ch.isSelected && !loadSelectedChannels ? 'disabled' : '';
        s += "<div> <button ondblclick='chId=this.id; openTagsPopup()' id='ch" + i + "' value='" + tagNames + "'>"
            + ch.name + "</button> <button id='ch" + i + "_sel' " + selDisabled + ">" + sel + "</button> </div>";
    }
    document.getElementById('main').innerHTML = s;
    setClickHandler();
}

function chSearch(s) {
    s = s.toUpperCase();
    for (const div of document.getElementById('main').getElementsByTagName('div')) {
        const gme = div.childNodes[1];
        div.style.display = gme.textContent.toUpperCase().indexOf(s) == -1 && gme.value.toUpperCase().indexOf(s) == -1 ? 'none' : 'block';
    }
}

var chId, chNum;

function openTagsPopup() {
    const id = chId.substring(2);
    ch = chans[id];
    chNum = ch.number;
    //T const tagIDs = [];
    const tagIDs = TagIDsForChan(chNum);
    document.getElementById('frmTagsSel').style.display = 'block';
    const chks = document.getElementById('frmTagsMain').getElementsByTagName('input');
    for (const chk of chks)
        chk.checked = tagIDs.includes(chk.id);
}

function TagsChansUpdate() {
    const chks = document.getElementById('frmTagsMain').getElementsByTagName('input');
    for (let i = 0; i < tags.length; i++) {
        if (chks[i].checked && !tags[i].channels.includes(chNum))
            tags[i].channels.push(chNum);
        if (!chks[i].checked && tags[i].channels.includes(chNum)) {
            const idx = tags[i].channels.indexOf(chNum);
            tags[i].channels.splice(idx, 1);
        }
    }
    const btn = document.getElementById(chId);
    btn.value = TagsForChan(chNum);
}
function closeTagsPopup() {
    document.getElementById('frmTagsSel').style.display = 'none';
}
function confirmTagsPopup() {
    TagsChansUpdate();
    var xhttp = new XMLHttpRequest();
    xhttp.open("POST", "/setTags", true);
    xhttp.setRequestHeader("Content-type", "text/plain");
    xhttp.send(tags.join('\n'));
    closeTagsPopup();
}