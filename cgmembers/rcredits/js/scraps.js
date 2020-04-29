/**
 * @file
 * javascript for the bottom of every page
 */

var vs = parseUrlQuery($('#script-scraps').attr('src').replace(/^[^\?]+\??/,'').replace('&amp;', '&'));
args = decodeURIComponent(vs['args']);
//alert(args);
args = JSON.parse(args);
for (var what in args) doit(what, parseUrlQuery(args[what]));

function doit(what, vs) {
  function fid(field) {return '#edit-' + vs[field].toLowerCase();}
  function fform(fid) {return $(fid).parents('form:first');}
  function report(j) {$.alert(j.message, j.ok ? 'Success' : 'Error');};
  function reportError(j) {if (!j.ok) $.alert(j.message, 'Error');};  

  switch(what) {

  case 'funding-criteria': jQuery('.critLink').click(function () {jQuery('#criteria').modal('show');}); break;
  case 'download': window.open(vs['url'] + '&download=1', 'download'); break;

  case 'chimp':
    var imp = $('.form-item-chimpSet');
    $('#edit-chimp-1').click(function () {imp.hide();});
    $('#edit-chimp-0').click(function () {imp.show();});
    break;

  case 'get-ssn': get('ssn', {}, function () {}); break;
    
  case 'deposits':
    $('.filename').click(function () {
      var area = document.createElement('textarea');
      area.value = $(this).attr('data-flnm');
      document.body.appendChild(area);
      area.select();
      alert(document.execCommand('copy') ? 'filename copied to clipboard' : 'copy to clipboard failed');
    });
    break;

  case 'summary':
    $('#activate-credit').click(function () {post('setBit', {bit:'debt', on:1}, report);});
    break;
    
  case 'invest':
    $('.form-item-expenseReserve a').click(function () {
      var reserve = parseFloat($('#edit-expensereserve').val().replace('$', ''));
      $('#edit-expensereserve').val(('$' + reserve).replace('NaN', '0'));
      post('set', {k:'minimum', v:reserve}, report);
    });
    break;
    
  case 'dollar-pool-offset':
    $('#dp-offset').click(function () {post('dpOffset', {amount:vs['amount']}, report);});
    break;
    

  case 'change-ctty':
    $('#edit-community').on('change', function () {
      var newCtty = this.value;
      changeCtty(newCtty, false);
    });

    function changeCtty(newCtty, retro) {
      /*        post('changeCtty', {newCtty:newCtty, retro:retro}, function(j) {
                if (!j.ok) $.alert(j.message, 'Error');
                }); */
      post('changeCtty', {newCtty:newCtty, retro:retro}, reportError);        
    }
    break;

  case 'focus-on': $('#edit-' + vs['field']).focus(); break;

  case 'agree':
    if (vs['show']) $('#wrap-agreement').show();
    $('#show-agreement').click(function () {$('#wrap-agreement').show();}); 
    break;
    
  case 'advanced-dates':
    if (!vs['showingAdv']) showAdv();
    $('#showAdvanced').click(function () {showAdv();});
    function showAdv() {jQuery('#advanced').show(); jQuery('#simple').hide();}
    $('#edit-period').change(function () {
      var id='#edit-submitPeriod'; if (!$(id).is(':visible')) id='#edit-downloadPeriod'; $(id).click();
    });
    $('#showSimple').click(function () {jQuery('#advanced').hide(); jQuery('#simple').show();});
    break;
    
  case 'new-advanced-dates':
    $('fieldset#dateRange #edit-advanced').click(function () {
      $('fieldset#dateRange #simple').hide();
      $('fieldset#dateRange #edit-advanced').hide();
      $('fieldset#dateRange #advanced').show();
      $('fieldset#dateRange #edit-simpler').show();
      $('fieldset#dateRange input#advOrSimple').value("advanced");
    });
    $('fieldset#dateRange #edit-simpler').click(function () {
      $('fieldset#dateRange #advanced').hide();
      $('fieldset#dateRange #edit-simpler').hide();
      $('fieldset#dateRange #simple').show();
      $('fieldset#dateRange #edit-advanced').show();
      $('fieldset#dateRange input#advOrSimple').value("advanced");
    });
    break;
    
  case 'paginate':
    $('#txlist #txs-links .showMore a').click(function () {showMore(0.1);});
    $('#txlist #txs-links .dates a').click(function () {
      $('#dateRange, #edit-submitPeriod, #edit-submitDates').show(); $('#edit-downloadPeriod, #edit-downloadDates, #edit-downloadMsg').hide();
    });
    $('#txlist #txs-links .download a').click(function () {
      $('#dateRange, #edit-downloadPeriod, #edit-downloadDates, #edit-downloadMsg').show(); $('#edit-submitPeriod, #edit-submitDates').hide();
    });
    $('#txlist #txs-links a.prevPage').click(function () {showPage(-1);});
    $('#txlist #txs-links a.nextPage').click(function () {showPage(+1);});
    page = 0;
    showPage(0);
    break;

  case 'reverse-tx':
    $('.txRow .buttons a[title="' + vs['title'] + '"]').click(function () {
      var url = this.href;
      yesno(vs['msg'], function () {location.href=url;}); 
      return false;
    });
    break;

  case 'signupco':
    $('#edit-agentqid').keyup(function () {reqQ($('.form-item-pass'), $('#edit-agentqid').val().trim() != '');});
    break;
    
  case 'cgbutton':
    cgbutton();
    $('#edit-item').focus();
    $('.form-item-button input').click(function () {cgbutton($(this).val());});
    $('#edit-item, #edit-text, #edit-amount, #edit-size').change(function () {cgbutton($('.form-item-button input:checked').val());});
    $('.form-item-for input').click(function () {
      var val = $(this).val();
      $('#edit-for').val(vs['forVals'].split(',')[val]);
      $('.form-item-item, .form-item-amount').toggle(val == 2);
      $(val == 2 ? '#edit-item' : '#edit-size').focus();
      cgbutton();
    });
    
    $('#edit-amount, #edit-size').keypress(function (e) {return '0123456789.'.indexOf(String.fromCharCode(e.which)) >= 0;});

    function cgbutton(type) {
      if (type == undefined) type = 2;
      var isButton = (type == 2);
      $('.form-item-size').toggle(isButton);
      $('.form-item-text').toggle(!isButton);
      $('.form-item-example').toggle(!isButton);
      
      var url = baseUrl + '/pay-with-cg';
      var fer = 'credit gift other'.split(' ')[$('input[name="for"]:checked').val()];
      var item = encodeURI($('#edit-item').val());
      var text = htmlEntities($('#edit-text').val());
      var size = $('#edit-size').val().replace(/\D/g, '');
      var amt = $('#edit-amount').val().replace(/\D/g, '');
      var img = isButton ? '<img src="https://cg4.us/images/buttons/cgpay.png" height="' + size + '" />' : text;
      var style = (type == 0 || type == 2) ? '' : ' style="display:inline-block; background-color:darkgreen; border-radius:5px; border:1px solid forestgreen; color:white; font-family:Arial; font-size:17px; padding:8px 15px; text-decoration-line:none;"';
      var html = vsprintf('<a href="%s/company=%s&for=%s&item=%s&amount=%s"%s target="_blank">%s</a>', [url, vs['qid'], fer, item, amt, style, img]);
      
      if ((isButton ? size : text) != '') {
        $('#edit-html').text(html);
        $('#button').html(html);
        $('.form-item-example .control-data').html(html);
      }
      $('.form-item-size img').height(size == '' ? 0 : size);
    }
    break;
    
  case 'addr':
/*    print_country(vs['country'], vs['state'], vs['state2']);
    $('#frm-signup, #frm-contact').submit(function () {
      $('#edit-hidcountry').val($('#edit-country').val());
      $('#edit-hidstate').val($('#edit-state').val());
      $('#edit-hidstate2').val($('#edit-state2').val());
    });
    $('.form-item-country select').change(function () {
      print_state(this.options[this.selectedIndex].value,'','state');
      print_state(this.options[this.selectedIndex].value,'','state2');
    });*/
    $('.form-item-sameAddr input[type="checkbox"]').change(function () {setPostalAddr(true);});
    break;

  case 'legal-name': // probably not needed with new signup system
    $('edit-fullname').change(function () {
      var legal=jQuery('#edit-legalname'); 
      if (legal.val()=='') legal.val(this.value);
    });
    break;
    
  case 'verifyid':
    if (vs['method'] >= 0) verifyid(vs['method']);
    $('[id^="edit-field-"]').click(function () {verifyid($(this).val());});
    $('#edit-file').click(function () {if ($(this).val() == '') $('#edit-field-0').prop("checked", true);});
    break;

  case 'which':
    var fid = fid('field');
    //      if ($(fid).val() == '') break; // don't suggest everyone
    var form = fform(fid);
    //      this.form.elements[vs['field']].value=this.options[this.selectedIndex].text;
    $('#which').modal('show');
    break;

  case 'suggest-who':
    var fid = fid('field');
    var form = fform(fid);
    suggestWho(fid, vs['coOnly']);
    $(fid).focus(); // must be after suggestWho
    form.submit(function (e) {
      if ($(fid).val() == '') return true; // in case this field is optional
      return who(form, fid, vs['question'], vs['amount'] || $('input[name=amount]', form).val(), vs['allowNonmember'], vs['coOnly']);
    });
    
    break;
    
  case 'new-acct':
    var fid = '#edit-newacct';
    var form = $('#frm-accounts');
    suggestWho(fid, '');
    form.submit(function (e) {
      return who(form, fid, '', false, true, false);
    });
    break;

  case 'invest-proposal':
    $('#add-co').click(function () {
      $('.form-item-fullName, .form-item-city, .form-item-serviceArea, .form-item-dob, .form-item-gross, .form-item-bizCats').show();
      require('#edit-fullname, #edit-city, #edit-servicearea, #edit-dob, #edit-gross, #edit-bizcats', true, true);
      $('.form-item-company').hide();
      require('#edit-company', false, true);
      $('#edit-fullname').focus();
    });
    
    $('#edit-equity-0, #edit-equity-1').click(function () {setInvestFields();});
    
//    setInvestFields($('#edit-equity').val() == 1);
      setInvestFields($('input[name="equity"]:checked').val() == 1);
    //      $('.form-item-equitySet').toggle($('#edit-equity').val());
    //      $('.form-item-loanSet').toggle(!$('#edit-equity').val());
    break;
    
  case 'advanced-prefs':
    toggleFields(vs['advancedFields'], false);
    $('#edit-showAdvancet').click(function () { $(this).hide(); toggleFields(vs['advancedFields'], true); });
    break;

  case 'bank-prefs':
    function showBank(show) {
      $('#connectFields2').toggle(show);
      require('#edit-routingnumber, #edit-bankaccount, #edit-bankaccount2, #edit-refills-0, #edit-refills-1', show, false);
      var text = show ? vs['connectLabel'] : vs['saveLabel'];
      $('#edit-submit, #edit-nextStep').val(text);
      $('#edit-submit .ladda-label, #edit-nextStep .ladda-label').html(text);
    }

    if ($('#edit-connect-2')[0]) {
      showBank($('#edit-connect-2').attr('checked') == 'checked');
      $('#edit-connect-0').click(function () {showBank(false);});
      $('#edit-connect-1').click(function () {showBank(false);});
      $('#edit-connect-2').click(function () {showBank(true);});
    } else if ($('#edit-connect-1')[0]) {
      showBank($('#edit-connect-1').attr('checked') == 'checked');
      $('#edit-connect-0').click(function () {showBank(false);});
      $('#edit-connect-1').click(function () {showBank(true);});
    }


    function showTarget(show) {
      $('#targetFields2').toggle(show);
      require('#edit-target, #edit-achmin', show, false);
    }
    showTarget($('#edit-refills-1').attr('checked') == 'checked');

    $('#edit-refills-0').click(function () {showTarget(false);});
    $('#edit-refills-1').click(function () {
      showTarget(true); 
      if ($('#edit-target').val() == '$0') $('#edit-target').val('$' + vs['mindft']);
    });
    break;

  case 'work':
    suggestWho('#edit-company', 1);
    break;
  
  case 'stepup':
    var first = true;
    
    $('[id^="edit-org"]').each(function () {
      if (!$(this).val()) {
        if (first) first = false; else $(this).parents('.row').hide();
      }
    });
    suggestWho('[id^="edit-org"]', 1);

    $('[id^="edit-org"]').change(function () {$(this).parents('.row').next().css('display', 'table-row');});
    
    $('[id^="edit-when"]').change(function () {
      var max = $(this).parents('.row').find('[id^="edit-max"]').parent();
      var pctmx = ($(this).find(':selected').val() == 'pctmx');
      max.toggle(pctmx);
      if (pctmx) {
        max.focus();
        $('.thead .cell.max').show();
      } else { max.val('').parent().parent().next().find('input[name^="org"]').focus(); }
    });
    
    $('[id^="edit-when"]').change();
        
    break;

  case 'groups':
    suggestWho('#edit-newmember');
    break;
    
  case 'rules':
    $('#edit').click(function () {location.href = baseUrl + '/sadmin/rules/id=' + $('#edit-id').val();});
    suggestWho('#edit-payer, #edit-payee, #edit-from, #edit-to');
    break;
    
  case 'posts':
    $('.form-item-radius .btn').click(function () { // click the Go button
      $('#edit-submit').click();
      return false; // cancel original link click
    });
    $('#menu-signin').hide(); // don't confuse (signin is not required for this feature)
    break;
    
  case 'post-tabs':
// NO. use URL instead  $('#edit-back').click(function () {window.history.back(); return false;});
    var frm = $('#edit-search').parents('form:first');
    
    frm.submit(function (event) {event.preventDefault();}); // or return false;
   
    $('#edit-search').change(function () { // search
      var box = $('#tabs .container');

      $('.filter').val(99); // show "(search)" on both filter dropdowns
      
      var s = $(this).val().trim().replace(/\s+/g, ' '); // the search string
      box.find('.tbody .row').show(); // show all (then eliminate non-matches)
      if (s == '') return $('.filter').val('');

      var i, words = s.toUpperCase().split(' '); // array of words
      var cols = '.cat .item .details'.split(' ');

      box.find('.tbody .row').each(function () {
        colText = ''; for (i in cols) colText += ' ' + $(this).find(cols[i]).text().toUpperCase();
        for (i in words) if (colText.indexOf(words[i]) < 0) $(this).hide(); // show only if it has all words
      });
    });

    $('#tabs').tabs();
    $('#tabs ul li a[href^="http"]').unbind('click').click(function () {location.href = $(this).attr('href');});
/*    $('[aria-controls="tab-needs"]').click(function () {$('#tab-needs').show();}); */
    $('#tabs .tbody .row').click(function () {location.href = $(this).find('a').attr('href');});
       
    $('.filter').mousedown(function () {$(this).parent().click();});
    
    $('.filter').change(function () {
      var box = $($(this).parent().attr('href'));
      var opt = $(this).find(':selected');

      if (opt.val() == -1) { // search
        $('#edit-search').change();
      } else if (opt.val()) { // show some
        var cat = opt.text().replace(/ /g, '');
        if (cat == vs['myPosts']) cat = 'mine';
        box.find('.tbody .row').hide(); // hide all
        box.find('.row.' + cat).show(); // and everything in the chosen category
        box.find('.row.none').show(); // and the "nothing found in this area" row, if any
      } else box.find('.tbody .row').show(); // show all
    });
    
    $('#edit-view').click(function () {
      if ($('#tabs.memo').length > 0) { // if memo view, switch to list view
        $('#tabs').removeClass('memo');
        $(this).text(vs['memoView']);
      } else { // list view, switch to memo
        $('#tabs').addClass('memo');
        $(this).text(vs['listView']);
      }
    });
    break;
    
  case 'post-post':
//    $('#edit-cat').change(function () {setCookie(vs['type'] + 'cat', $(this).val());});
    $('input[name="type"]').change(function () {
      var type = vs['types'].split(' ')[$(this).val()];
      var need = (type == 'need');
      $('.form-item-radius').toggle(!need); 
      $('.form-item-exchange').toggle(need);
      if (type == 'tip') $('#edit-radius').val(0); // tips default to everywhere
    });
    
    $('.form-item-end a').click(function () {
      $('#edit-end').val(new Date(Date.now()).toLocaleString().split(',')[0]);
      $('#edit-submit').focus();
    });
    break;

  case 'post-who':
    $('#edit-fullname').change(function () {
      var nick = $('#edit-displayname');
      if (!nick.val()) nick.val($(this).val().trim().split(' ')[0]);
    });
  /*
    $('#edit-zip').change(function () {
      var moderate = (vs['moderateZips'].indexOf($(this).val().trim().substring(0, 3)) >= 0);
      var m, a = 'midtext days washes health'.split(' ');
      for (i in a) {
        m = $('.form-item-' + a[i]);
        m.toggle(moderate);
        if (moderate) {
          m.find('input').attr('required', 'required');
        } else m.find('input').removeAttr('required');
      }
    }); */
    break;
    
  case 'signup':
    var form = $('#frm-signup');
//    if (vs['clarify'] !== 'undefined') $('#edit-forother a').click(function () {alert(vs['clarify']);});
    form.submit(function (e) {return setPostalAddr(false);});
    if (vs['preid']) $('#edit-phone').change(function () {
      data = {
        preid: vs['preid'],
        fullName: $('#edit-fullname').val(),
        legalName: $('#edit-legalname').val(),
        email: $('#edit-email').val(),
        phone: $('#edit-phone').val()
      };
      post('presignup', data, null);
    });
    break;
    
  case 'prejoint': $('#edit-old-0').click(function () {this.form.submit();}); break;

  case 'invite-link': $('#inviteLink').click(function () {SelectText(this.id);}); break;

  case 'amtChoice':
    var other = jQuery('.form-item-amount'); 
    var amtChoice = jQuery('#edit-amtchoice');
    var amtChoiceWrap = jQuery('.form-item-amtChoice');
    if (amtChoice.val() == -1) {
      amtChoiceWrap.hide();
      other.show(); 
    } else {
      amtChoiceWrap.show();
      other.hide();
    }
    
    amtChoice.click(function () {
      if(amtChoice.val() == -1) {
        other.show(); 
        amtChoiceWrap.hide();
        jQuery('#edit-amount').focus();
      } else other.hide();
    });
    
    $('#edit-amount').change(function () {
      if ($(this).val() == 0) $('#edit-often').val('Y');
    });
    break;
    
  case 'contact':
    var form = $('#frm-contact');
    $('#edit-fullname', form).focus();
    $('#edit-email', form).change(function () {$('.form-item-pass').show(); $('#edit-pass').focus();});
    form.submit(function (e) {return setPostalAddr(false);});
    break;

  case 'verifyemail': // have to use jQuery here instead of $ because of Drupal conflict
    if (vs['verify'] == 1) {
      reqNot(jQuery('.form-item-pass1'));
      reqNot(jQuery('.form-item-pass2'));
    } else showPassFlds();
    jQuery('#edit-showpass-1').click(showPassFlds);
    break;
    
  case 'veto':
    $('.veto .checkbox input').change(function () {
      var opti = this.name.substring(4);
      opts[opti].noteClick();
    });
    break;
    
  case 'back-button': $('.btn-back').click(function () {history.go(-1); return false;}); break;
    
  case 'tickle': 
    $('.tickle').click(function () {
      var tickle = $(this).attr('tickle');
      $('#edit-tickle').val(tickle);
      $('#edit-submit').click();
    });
    break;
    
  case 'coupons':
    var purposeDft = $('#edit-purpose').val();
    $('#edit-minimum').change(function () {
      var min = $(this).val();
      $('#edit-purpose').val(min == '0' ? purposeDft : vs['minText'].replace('%min', min));
    });

    $('#edit-automatic-0').click(function () {
      $('.form-item-automatic').hide();
      $('.form-item-purpose').show();
    });
    break;
    
  case 'dispute':
    $('#dispute-it').click(function () {
      $('#denySet').show(); 
      $('.form-item-pay').hide();
    });
    break;

  case 'followup-email':
    $('#email-link').click(function () {
      var L = $(this);
      L.html('<h3 style="color:red;">Press Ctrl-C, Enter (then Ctrl-V in email)</h3>');
      var d=$('#email-details');
      d.attr('tabindex', '99'); // oddly, Chrome requires this for keydown
      d.attr('class', ''); // show
      d.focus();
      SelectText(d[0].id);
      d.bind('keydown', function(event) {
        if (event.keyCode!=13) return;
        $('#email-do')[0].click();
        d.attr('class', 'collapse'); // hide
        L.html('email');
      });
    });
    break;

  case 'invoices':
    $('#txlist tr td').not('#txlist tr td:last-child').click(function () {
      var nvid = $(this).siblings().first().html();
      location.href = baseUrl + '/handle-invoice/nvid=' + nvid + vs['args'];
    });
    break;
    
    /*    case 'relations':
          $('div.checkbox').click(function () {
          var box = $('input', this);
          alert(box.prop('checked'));
          //box.prop('checked', !box.prop('checked'));
          });
          break;*/
    
  default:
    alert('ERROR: Unknown script scrap (there is no default script).');
    alert($('#script-scraps').attr('src').replace(/^[^\?]+\??/,''));
    
  }
}

/**
 * Show or hide ID verification fields according to verification method
 */
function verifyid(method) {
  $('#edit-field-' + method).prop('checked', true); // needed after error
  var ssn = $('.form-item-federalId');
  var dob = $('.form-item-dob');
  var idtype = $('.form-item-idtype');
  var file = $('.form-item-file');
  if (method == 0) {reqNot(ssn); reqNot(dob); req(file); reqNot(idtype);}
  if (method == 1) {reqNot(ssn); reqNot(dob); req(file); req(idtype); $('#edit-idtype').focus();}
  if (method == 2) {req(ssn); req(dob); reqNot(file); reqNot(idtype); $('#edit-federalid').focus();}
}  

function setInvestFields() {
  var equity = ($('input[name="equity"]:checked').val() == 1);
  $('.form-item-equitySet').toggle(equity);
  $('.form-item-loanSet').toggle(!equity);
  require('#edit-offering, #edit-price, #edit-return', equity, true);
  require('#edit-offering--2, #edit-price--2, #edit-return--2', !equity, true);
}

/**
 * Require or don't the given fields.
 * @param set items: a jQuery selector
 * @param bool yesno: set or don't
 * @param bool xx: change name of non-required fields to xx-name (and remove the xx- for required)
 */
function require(items, yesno, xx) {
  if (yesno) {
    $(items).prop('required', true);
    if (xx) $(items).each(function (i) {
      $(this).attr('name', $(this).attr('name').replace('xx-', ''));
    });
  } else {
    $(items).removeAttr('required');
    if (xx) $(items).each(function (i) {
      $(this).attr('name', 'xx-' + $(this).attr('name'));
    });    
  }
}

function showPassFlds() {
  jQuery('.form-item-pass1,.form-item-pass2,#edit-settings').show();
  jQuery('#edit-pass1').focus();
}
  
function req(fld) {fld.show(); fld.find('input').attr('required', 'required');}
function reqNot(fld) {fld.find('input').removeAttr('required'); fld.hide();}
function reqQ(fld, show) {if(show) req(fld); else reqNot(fld);}