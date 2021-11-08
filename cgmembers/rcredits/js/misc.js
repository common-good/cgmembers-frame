/**
 * @file
 * javascript for the bottom of every page
 */

var vs = parseUrlQuery($('#script-misc').attr('src').replace(/^[^\?]+\??/,''));
//alert($('#script-misc').attr('src').replace(/^[^\?]+\??/,''));
var baseUrl = vs['baseUrl'];
var isSafari = vs['isSafari'];
var signoutUrl = baseUrl + '/signout/timedout';
var ajaxUrl = baseUrl + '/ajax';
var ajaxSid = vs['sid'];
var sessionLife = 1000 * vs['life']; // convert to seconds (sessionLife is 0 if not signed in -- no expiration)
var signoutWarningAdvance = Math.min(sessionLife / 2, 3 * 60 * 1000); // give the user a few minutes to refresh
if (ajaxSid) var sTimeout = sessionLife == 0 ? 0 : sessionTimeout(); // Warn the user before automatically signing out.

if (sessionLife == 0 && 'serviceWorker' in navigator) navigator.serviceWorker.register(baseUrl + '/sw.js');
  
$('.showMore').click(function () {$('.more').show(); $(this).hide();});
$("#which, #help").addClass("popup");
$('button[type="submit"]').click(function() {this.form.opid.value = this.id;});

$('[data-toggle="popover"][data-trigger="hover"]').click(function () {$(this).popover('toggle');});
$('.submenu .popmenu a').click(function () {$(this).find('.glyphicon').css('color', 'darkblue');});
$('.submenu a[data-trigger="manual"]').click(function () {
//  if (isSafari) location.href = baseUrl + '/' + $(this).parents('.submenu').attr('id').replace('menu-', ''); // work around Safari bug (doesn't show menus on hover)
  $(this).popover('toggle');
  $('.submenu a').not($(this)).popover('hide');
});
$('#extras button img').click(function () {$('#edit-accounts').toggle(); $('#edit-newacct').focus();}); // show outer div first

var page=0;
var more=false;
var indexZ = 2;
jQuery("#index a").mouseover(function() {
  var detail = jQuery("#" + this.id + "-detail");
  indexZ++;
  detail.css("zIndex", indexZ); // hiding the others fails here (as does detail.zIndex(indexZ))
  detail.show();
});
jQuery(".index-detail").click(function() {jQuery("#edit-acct-index, .index-detail").hide();});
jQuery(".noEdit").prev().attr("disabled", 1);

jQuery('[data-toggle="popover"][data-trigger="hover"]').popover(); 
jQuery('[data-toggle="popover"][data-trigger="click"]').popover(); 

var mobile = jQuery('.navbar-toggle').is(':visible');
jQuery('.submenu [data-toggle="popover"]').each(function(index) {
  jQuery(this).popover({
    html: true,
    content: function() {return jQuery(this).prev().html();},
    placement: (mobile ? 'left' : 'bottom')
  });
});
jQuery('#main .list-group-item.ladda-button').attr('data-spinner-color', '#191970').click(function() {
  jQuery(this).find('.glyphicon').css('color', 'white');
});
if (Ladda != null) Ladda.bind('.ladda-button');
if (!mobile) jQuery('.navbar-nav > li > a').hover(function() {
  jQuery(this).popover('show');
  if (Ladda != null) Ladda.bind('.ladda-button'); // these buttons are not available to Ladda until now
  // ('#' + jQuery(this).parent().parent().attr('id') + ' > li > a') doesn't work
  jQuery('.submenu > a').not(jQuery(this)).popover('hide');
});
if (!mobile) jQuery('form div').hover(function() {jQuery('* [data-toggle="popover"]').popover('hide');});

$('.form-horizontal :not([class="invisible"]):input:enabled:visible:first:not([tabindex="-1"])').focus();

$('.test-next').click(function () {
  $('#testError' + $(this).attr('index'))[0].scrollIntoView(true); window.scrollBy(0, -100);
});

$('[class^="qbtn-"]').click(function () {
  var pop = $('#help-modal');
  var cl = $(this).attr('class');
  post('qBtn', {topic:cl.substring(cl.indexOf('-') + 1)}, function (j) {
    pop.find('.modal-title').html(j.title);
    pop.find('.modal-body').html(j.body);
    pop.modal('show');
  });
});

function showMore(pgFactor) {
  page = Math.floor(page * pgFactor); 
  if (more) {
    $.alert('Click &#9654; (far right) to see the next page', 'Tip');
  } else {
    more = true;
    if ($('.PAGE-' + (page + 1)).length) {
      $('.showMore a').css('color','silver'); 
    } else $('.showMore').css('visibility','hidden'); 
  }
  showPage(0);
}

function showPage(add) {
  page += add;
  var pghd = more ? '.PAGE-' : '.page-'; 
  $('.prevPage').css('visibility', page < 1 ? 'hidden' : 'visible'); 
  $('.nextPage').css('visibility', $(pghd + (page + 1)).length ? 'visible' : 'hidden');
  var rows = $('tr.txrow');
  var thisPage = rows.filter(more + page).show();
  rows.not(thisPage).hide();
  $('.txRow').hide(); 
  $('.txRow.head, ' + pghd + page).show();
}

function deleteCookie(name) {
  document.cookie = name + '=; expires=Thu, 01 Jan 1970 00:00:01 GMT;';
}

function toggleFields(fields, show) {
  fields.split(' ').forEach(function(e) {$('.form-item-' + e).toggle(show); });
}

function toggle(field) {
  field = "#" + field;
  jQuery(field + "-YES, " + field + "-NO").toggle().toggleClass("visible invisible");
  jQuery(field).val(jQuery(field + "-YES").is(":visible"));
}

function commafy(n) {return isNaN(n) ? '0.00' : n.toLocaleString();}

/**
 * post or get data to/from the server
 * @param string op: what to get
 * @param object data: parameters for the get
 * @param function success(jsonObject): what to do upon success (do nothing on failure)
 */
function get(op, data, success) {
  data = {op:op, sid:ajaxSid, data:JSON.stringify(data)}; // sub-objects must be stringified
  jQuery.get(ajaxUrl, data, success);
}

function post(op, data, success) {
  data = {op:op, sid:ajaxSid, data:JSON.stringify(data)};
  jQuery.post(ajaxUrl, data, success); // jQuery not $, because drupal.js screws it up on formVerify
}

function yesno(question, yes, no) {return confirm0('Yes or No', question, 'Yes', 'No', yes, no);}
function confirm(title, question, yes, no) {return confirm0(title, question, 'Ok', 'Cancel', yes, no);}

function confirm0(title, question, labYes, labNo, yes, no) {
  if (title === null) title = 'Confirm';
  if (typeof no === 'undefined') no = function() {};
  return $.confirm({title: title, text: question, confirm: yes, cancel: no, confirmButton: labYes, cancelButton: labNo});
}

var yesSubmit = false; // set true when user confirms submission (or makes a choice)
var jForm; // jquery form object

function noSubmit() {
  $('.ladda-button').removeAttr('disabled').removeAttr('data-loading');
  $('#messages').hide();
}
function yesSubmit() {}

/**
 * Find out what account the user means (see w\whoFldSubmit).
 * Create a hidden whoId field to store the record Id.
 */
function who(form, fid, question, amount, selfErr, restrict, allowNonmember) {
  jForm = $(form);
  var who = $(fid).val();
  if (yesSubmit) return true;
  get('who', {who:who, question:question, amount:amount, selfErr:selfErr, restrict:restrict}, function(j) {
    if (j.ok) {
      if (j.who) {
        setWhoId(j.who, jForm);
        
        if (j.confirm != '') {
          yesno(j.confirm, function() {
            yesSubmit = true; jForm.submit();
          }, noSubmit);
        } else {yesSubmit = true; jForm.submit();}
      } else which(jForm, fid, j.title, j.which);
    } else if (allowNonmember == 1 && who.includes('@') && fid != '#edit-newacct') {
      yesno('The email address (' + who + ') is for a non-member (or for a member with a non-public email address). ' + question.replace('?', '').replace('%amount', fmtAmt(amount)).replace('%name', who) + ' anyway, with an invitation to join?', function() {
        yesSubmit = true; jForm.submit();
      }, noSubmit);
    } else {
      noSubmit(); $.alert(j.message);
    }
  });
  return false;
}

function setWhoId(id, frm) {
  var whoId = $('input[name="whoId"]', frm);
  if (whoId.length > 0) { // save record ID in hidden field, creating if necessary
    whoId.val(id);
  } else frm.append('<input type="hidden" name="whoId" value="' + id + '" />');
}

function which(jForm, fid, title, body) {
  $('<div id="which">' + body + '</div>').dialog({
    title: title,
    modal: true,
    closeText: '&times;', // fails
    dialogClass: 'which'
  });
  $('.ui-dialog-titlebar-close').html('&times;');
  $('.ui-dialog-titlebar-close').click(function () {noSubmit();});

  $('#which option').click(function () {clickWhich(fid, $(this).val(), $(this).text(), jForm);});
  
  $('#which select').keypress(function (e) {
    if (e.which != 13) return;
    var id = $(this).val();
    var text = $(this).find('option[value="' + id + '"]').text();
    clickWhich(fid, id, text, jForm);
  });
}

function clickWhich(fid, id, text, frm) { 
    yesSubmit = true;
    $(fid).val(text);
    setWhoId(id, frm);
    $('#which').hide();
    frm.submit();
}

/**
 * Activate typeahead functionality for an account input element.
 * @param string sel: input element selector
 * @param string restrict: MySQL restrictions, if any
 */
function suggestWho(sel, restrict) {
  var members = new Bloodhound({
  //  datumTokenizer: Bloodhound.tokenizers.obj.whitespace('value'),
    datumTokenizer: Bloodhound.tokenizers.whitespace,
    queryTokenizer: Bloodhound.tokenizers.whitespace,
    prefetch: {
      url: ajaxUrl + '?op=suggestWho&data={"restrict":"' + restrict + '"}&sid=' + ajaxSid,
      cache: false
    }
  });
  $(sel).wrap('<div></div>').typeahead(
    {minLength: 3, highlight: true},
    {name: 'cgMembers', source: members} //    display: 'value',
  );
}
    
var signoutWarning = 'You still there? (otherwise we\'ll sign you out, to protect your account)';

function sessionTimeout() {
  return setTimeout(function() {
    $.confirm({
      title:'Long Time No Click',
      text:signoutWarning,
      confirmButton:'Yes',
      cancelButtonClass:'hidden',
      confirm:function() {
        clearTimeout(sTimeout); // don't sign out
        $.get(ajaxUrl, {op:'refresh'}); // reset PHP garbage collection timer
        sTimeout = sessionTimeout(); // restart warning timer
      }
    });
    sTimeout = setTimeout(function() {location.href = signoutUrl;}, Math.max(1, signoutWarningAdvance - 10));
  }, sessionLife - signoutWarningAdvance);
}

function SelectText(element) { // from http://stackoverflow.com/questions/985272
  var doc = document;
  var text = doc.getElementById(element);
  var range, selection;
  if (doc.body.createTextRange) {
    range = doc.body.createTextRange();
    range.moveToElementText(text);
    range.select();
  } else if (window.getSelection) {
    selection = window.getSelection();        
    range = doc.createRange();
    range.selectNodeContents(text);
    selection.removeAllRanges();
    selection.addRange(range);
  }
}

/**
 * Copy specified value to clipboard.
 * @param string s: string to copy
 * @return <success>
 */
function clipCopy(s) {
  var area = document.createElement('textarea');
  area.value = s;
  document.body.appendChild(area);
  area.select();
  return document.execCommand('copy');
}

function getCookie(name) {
  var v = document.cookie.match('(^|;) ?' + name + '=([^;]*)(;|$)');
  return v ? v[2] : null;
}

/**
 * Set a cookie. Use exdays=99999 for "never expires"
 */
function setCookie(cname, cvalue, exdays) {
    var d = new Date();
    d.setTime(d.getTime() + (exdays*24*60*60*1000));
    var expires = "expires="+ d.toUTCString();
    document.cookie = cname + "=" + cvalue + ";" + expires + ";path=/";
}

function vsprintf(s, args) {
  var res = s;
  for (var k in args) res = res.replace('%s', args[k]);
  return res;
}

function htmlEntities(str) {
    return String(str).replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;').replace(/"/g, '&quot;');
}

function fmtAmt(n) {
  var res = (Math.round((parseFloat(n) + Number.EPSILON) * 100) / 100).toLocaleString(undefined, {maximumFractionDigits:2});
  return (has(res, '.') && res.indexOf('.') == res.length - 2) ? res + '0' : res;
}

/* function fmtAmt(n, minDigs, maxDigs) {
  if(typeof minDigs == undefined) minDigs = 2;
  if(typeof maxDigs == undefined) maxDigs = 2;
  return n.toLocaleString(undefined, {minimumFractionDigits:minDigs, maximumFractionDigits:maxDigs}); // maximumFractionDigits fails in Safari/Firefox
} */

function has(hay, needle) {
  return ((hay + '').indexOf(needle + '') >= 0);
}
/**
 * Build new jQuery syntax: $(":icontains['Bozo']") selects all elements containing "bozo", case-insensitive.
 */ /* FAILS
 
 //      selector = ".cell:icontains['" + s.split(" ").join("']:icontains['") + "']";
//      box.find(selector).show();

jQuery.expr[':'].icontains = function(a, i, m) {
  return jQuery(a).text().toUpperCase()
      .indexOf(m[3].toUpperCase()) >= 0;
}; */