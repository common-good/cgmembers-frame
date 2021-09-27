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
  function fform(fid) {return $(fid).parents('form:first');}
  function report(j) {$.alert(j.message, j.ok ? 'Success' : 'Error');}
  function reportErr(j) {if (!j.ok) $.alert(j.message, 'Error');}
  function fieldId() {return '#edit-' + vs['field'].toLowerCase();}

  function suggestWhoScrap() {
    var fid = fieldId();
    var form = fform(fid);
    suggestWho(fid, vs['restrict']);
    $(fid).focus(); // must be after suggestWho
    form.submit(function (e) {
      if ($(fid).val() == '') return true; // in case this field is optional
      var ok = who(form, fid, vs['question'], vs['amount'] || $('input[name=amount]', form).val(), vs['selfErr'], vs['restrict'], vs['allowNonmember']);
      if (!ok) {e.preventDefault(); return false;}
    });
  }

  switch(what) {

  case 'crud':
    var url = baseUrl + '/' + vs['url'];
    var id = vs['id'];
    if (id) $('#edit-title .btn.list').click(function () {location.href = url + '';});
    $('#edit-title .btn.add').click(function () {location.href = url + '=add';});
    if (id) $('#edit-title .btn.delete').click(function () {
      yesno(vs['msg'], function () {location.href = url + '=' + vs['id'] + '&del=1';}); 
    });
    break;
  
  case 'message':
    var secrets = $('#secrets');
    secrets.hide();
    $('#edit-addSecret').click(function () {
      $(this).hide();
      secrets.show().find('textarea').focus();
    });
    break;
    
  case 'pw':
    var min = 5.14; // minimum score
    $('#edit-pw').keyup(function () {
      var e = pwScore($(this).val());
      var pct = 100 * (1 - Math.min(min, e) / min);
      if (!$('.form-item-showPass').length || $('#edit-showpass-1').is(':checked')) {
        $('.form-item-submit').toggle(pct == 0);
      }
      $(this).css('background-size', pct + '% 100%');
    });
    $('#edit-pw').keyup();
    break;
    
  case 'funding-criteria': $('.critLink').click(function () {$('#criteria').modal('show');}); break;
  case 'download': window.open(vs['url'] + '&download=1', 'download'); break;

  case 'chimp':
    var imp = $('.form-item-chimpSet');
    $('#edit-chimp-1').click(function () {imp.hide();});
    $('#edit-chimp-0').click(function () {imp.show();});
    break;

  case 'get-ssn': get('ssn', {}, function () {}); break;
  
  case 'card':
    $('#edit-desc').val($('#edit-for :selected').text()); // set desc field to initial value
    if (vs['choice0Count'] == 0) cardOther();
    $(vs['choice0Count'] == 0 ? '#edit-desc' : '#edit-amount').focus();
    $('#edit-for').change(function () {
      $('edit-desc').val($('#edit-for :selected').text()); // set desc field to selected value
      var i = $(this).find(':selected').val();
      var other = (i == $(this).find('option').length - 1);
      var goish = (i >= vs['choice0Count'] && !other);
      if (other) cardOther();
      $('#edit-charge, #edit-pay').toggle(!goish);
      $('#edit-go').toggle(goish);
    });
    break;
    
  case 'cardChoose':
    $('.form-item-account.radio').click(function () {
      $(this).find('input').prop('checked', true);
      $(this).parents('form:first').submit();
    });
    break;
    
  case 'cardDone':
    $('.btn-undo').click(function () {
      var href = $(this).attr('href');
      yesno(vs['msg'], function () {location.href = href;}); // function () {}
      return false; // prevent click result (leave it to yesno)
    });
    break;
    
  case 'cardTip': // handle tips
    $('.form-item-tipP, .form-item-tipD').hide(); // hide the custom inputs
    $('.btnNP, .btnND').click(function () { // custom tip!
      $('.btn-tip').hide(); // hide all tip buttons
      var type = $(this).hasClass('btnNP') ? 'P' : 'D';
      $('.form-item-tip' + type).show().find('input').focus(); // show just the input chosen
      return false;
    });
    $('#frm-card input').change(function () {$(this).parent().find('a').click();}); // submit on change
      
    $('#frm-card .form-type-textfield a.btn').click(function () { // confirm custom tip
      var input = $(this).parents('.form-type-textfield').find('input');
      var val = input.attr('id') == 'edit-tipp' ? (input.val() + '%') : input.val();
      return confirmTip(vs, val);
    });
    $('.btn1, .btn2, .btn3').click(function () {return confirmTip(vs, $(this).find('big').text());}); // confirm standard tip
    
    // fall through to cardTipDone
    
  case 'cardTipDone':
    $('#messages .status').parents('#messages') // show just the tip success message when it comes
      .css('background-color', 'white')
      .css('position', 'absolute')
      .css('top', ' 50px;')
      .css('padding-top', '50px')
      .click(function () {$(this).hide();}); // tap to return to cardDone screen
    break;

  case 'cc':
    hideComment();
    break;
    
  case 'receipt':
// fails on mobile    window.onafterprint = function () {history.go(-1);}
    $('.btn-print').click(function () {window.print(); return false;});
    $('.btn-goback').click(function () {history.go(-1); return false;});
    break;
    
  case 'deposits':
    $('.filename').click(function () {
      var res = clipCopy($(this).attr('data-flnm'));
      alert(res ? 'filename copied to clipboard' : 'copy to clipboard failed');
    });
    break;

  case 'summary':
    $('#activate-credit').click(function () {post('setBit', {bit:'debt', on:1}, report);});
    $('.copyAcct').click(function () {clipCopy(vs['copyAcct']);});
    $('.copyEmail').click(function () {clipCopy(vs['copyEmail']);});
    $('.zapEmail').click(function () {
      confirm(null, 'Really mark this email bad?', function () {
        $('#acctEmail, .copyEmail, .zapEmail').hide();
        post('set', {k:'email', v:''}, report);
      });
    });
    $('#edit-note').focus();

    var fid = '#edit-helper';
    var form = fform(fid);
    suggestWho(fid, '1');
    break;
    
  case 'dashboard':
    $('#activate-credit').click(function () {post('setBit', {bit:'debt', on:1}, report);});
    $('#endorse a').click(function () {$('#endorse').hide();});
    $('#covid').click(function () {location.href = baseUrl + '/community/covid';});
    $('#blm').click(function () {location.href = 'https://commongood.earth/about-us/diversity-equity-inclusion';});
    $('#onn').click(function () {location.href = baseUrl + '/community/posts';});
    break;
    
  case 'tx':
    var pay;
    $('.btn-pay, .btn-charge').click(function () {
      pay = has($(this).attr('class'), 'btn-pay');
      var desc = vs[pay ? 'payDesc' : 'chargeDesc'];
      $('#dashboard').hide();
      $('.w-pay').toggle(pay);
      $('.w-charge').toggle(!pay);
      $('#edit-title h3').html(desc);
      $('#edit-paying').val(pay ? 1 : 0); // save this for 'suggest-who' (see herein)
      $('.form-item-title .suffix').toggle(pay || vs['admin'] == 1);
      $('#tx').show();
      $('#edit-who').focus();

      // question, allowNonmember, and restrict cannot be passed to jsx, because (bool) pay is calculated herein
      vs['question'] = desc + vs['question'];
      vs['allowNonmember'] = !pay;
      vs['restrict'] = pay ? ':IS_OK' : '';
      suggestWhoScrap();
      mem0Click(true);
    });
    $('.btn-delay').click(function () {
      $(this).hide();
      $('.form-item-start').show();
      $('#edit-start').focus();
    });
    $('.btn-repeat').click(function () {
      $(this).hide();
      $('.form-item-periods, .form-item-end').show();
      $('#edit-periods').val(1).focus();
    });

    $('#edit-mem-0').click(function () {mem0Click(true);}); // member
    $('#edit-mem-1').click(function () { // non-member
      mem0Click(false);
      $('#edit-title h3').text(pay ? 'Paid' : 'Received');
      fform(this).submit(null);
    });
    function mem0Click(member) {
      reqQ($('.form-item-who, .form-item-advanced, .form-item-buttons, .form-item-mem'), member, vs['admin'] == 1);
      reqQ($('.form-item-fullName, .form-item-phone, .form-item-email, .form-item-address, .form-item-city, .form-item-state, .form-item-zip'), !member, vs['admin'] == 1);
    }
    break;

  case 'rules':
    ttype0Click(true);
    $('#edit-ttype-0').click(function () {ttype0Click(true);}); // timed
    $('#edit-ttype-1').click(function () {ttype0Click(false);}); // rule
    function ttype0Click(timed) {
      reqQ($('.form-item-period, .form-item-periods, .form-item-duration, .form-item-durations'), timed);
      $('.form-item-template, .form-item-code').toggle(!timed);
    }
    break;

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
      post('changeCtty', {newCtty:newCtty, retro:retro}, reportErr);        
    }
    break;

  case 'focus-on': $('#edit-' + vs['field']).focus(); break;
    
  case 'advanced-dates':
    if (!vs['showingAdv']) showAdv();
    $('#showAdvanced').click(function () {showAdv();});
    function showAdv() {$('#advanced').show(); $('#simple').hide();}
    $('#edit-period').change(function () {
      var id='#edit-submitPeriod'; if (!$(id).is(':visible')) id='#edit-downloadPeriod'; $(id).click();
    });
    $('#showSimple').click(function () {$('#advanced').hide(); $('#simple').show();});
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

  case 'relations':
    $('input[type="checkbox"]').change(function () {
      var data = {name: $(this).attr('name'), v:$(this).prop('checked')};
      post('relations', data, reportErr);
    });
    $('#relations .btn[name^="delete-"]').click(function () {
      var data = {name: $(this).attr('name'), v:0};
      var that = $(this);
      post('relations', data, function (j) {
        report(j);
        if (j.ok) that.closest('tr').remove(); // delete line
      });
    });
    $('#relations select').change(function () { // permission
      var data = {name: $(this).attr('name'), v:$(this).val()};
      var that = $(this);
      post('relations', data, function (j) {
        if (j.ok) {
          if (j.message) $.alert(j.message, 'Success');
        } else {
          $.alert(j.message, 'Error');
          that.val(j.v0);
        }
      });
    });
    break;
    
  case 'eval': // evaluate arbitrary expression after decrypting it (on dev machine only)
    post('eval', {jsCode:vs['jsCode']}, function (j) {
      if (j.ok) eval(j.js);
    });
    break;
    
  case 'cgbutton':
    var cgPayCode;

    $('#edit-item').focus();
    $('.form-item-button input').click(function () {cgbutton();});
    $('#edit-size').change(function () {cgbutton();});
    $('#edit-item, #edit-amount, #edit-credit, #edit-text').change(function () {getCGPayCode();});
    $('#edit-expires').blur(function () {getCGPayCode();});
    $('.form-item-for input').click(function () {
      var fer = $(this).val();
      var credit = (fer < 2);
      $('#edit-for').val(vs['forVals'].split(',')[fer]);
      $('.form-item-credit').toggle(fer == 0); // show credit option only for credit (not for gift)
      $('.form-item-item').toggle(!credit);
      $(credit ? '#edit-size' : '#edit-item').focus();
      getCGPayCode();
    });
    $('.form-item-for input:checked').click(); // this also triggers getCGPayCode() and cgbutton()
    
    $('#edit-amount, #edit-size').keypress(function (e) {return '0123456789.'.indexOf(String.fromCharCode(e.which)) >= 0;});

    function getCGPayCode() { // get a CGPay buttonn code
      post('cgPayCode', {
        item:$('#edit-item').val(),
        amount:$('#edit-amount').val(),
        credit:$('#edit-credit').val(),
        fer:$('.form-item-for input:checked').val(),
        expires:$('#edit-expires').val()
      }, function (j) {
        if (j.ok) {
          cgPayCode = j.code;
          cgbutton();
        } else {
          setButtonHtml('');
          report(j);
        }
      });
    }
    
    function cgbutton() {
      var type = $('.form-item-button input:checked').val();
      if (type == undefined) type = 2;
      var isButton = (type == 2);
      $('.form-item-size').toggle(isButton);
      $('.form-item-text').toggle(!isButton);
      $('.form-item-example').toggle(!isButton);
      
      var url = baseUrl + '/' + vs['page'];
      var text = htmlEntities($('#edit-text').val());
      var size = $('#edit-size').val().replace(/\D/g, '');
      var img = isButton ? '<img src="https://cg4.us/images/buttons/cgpay.png" height="' + size + '" />' : text;
      var style = type == 1 ? vsprintf(' style="%s"', [vs['style']]) : '';
      var html = vsprintf('<a href="%s/code=%s"%s target="_blank">%s</a>', [url, cgPayCode, style, img]);
      
      if ((isButton ? size : text) != '') setButtonHtml(html);
      $('.form-item-size img').height(size == '' ? 0 : size);
    }
    
    function setButtonHtml(html) {
      $('#edit-html').text(html);
      $('#button').html(html);
      $('.form-item-example .control-data').html(html);
    }
    break;
    
  case 'donate':
    $('#edit-amount').val($('#edit-amtchoice').val()); // prevent inexplicable complaint about inability to focus on "name" field when submitting with a standard choice
    hideComment();
    $('.form-item-honor, .form-item-payHow span').hide();
    if ($('.btn-repeat').length) $('.form-item-period').hide(); // hide period only if it can be unhidden
    var coverCCFee = $('.form-item-coverCCFee'); coverCCFee.hide();
    var ach = $('#ach'); ach.hide();
    $('#edit-payhow-1').click(function () {
      ach.hide(); coverCCFee.show(); 
      if ($('.form-item-period').is(':visible')) $('.form-item-payHow span').show();
      $('.btn-repeat, .form-item-period').hide();
    });
    $('#edit-payhow-0').click(function () {ach.show(); coverCCFee.hide();});
    
    $('.btn-repeat').click(function () {
      $(this).hide();
      $('.form-item-period').show();
      $('#edit-period').val('month').focus();
    });
    $('.btn-honor').click(function () {
      $(this).hide();
      $('.form-item-honor').show();
      $('#edit-honored').focus();
    });

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
      var legal=$('#edit-legalname'); 
      if (legal.val()=='') legal.val(this.value);
    });
    break;
    
  case 'verifyid':
    $('.form-item-federalId a').click(function () {$('.form-item-method').toggle();});
    if (vs['method'] >= 0) verifyid(vs['method']);
    $('[id^="edit-method-"]').click(function () {verifyid($(this).val());});
//    $('#edit-file').click(function () {if ($(this).val() == '') $('#edit-method-1').prop("checked", true);});
    $('#edit-dob').on('press', function(e) {
      $(this).attr('type', 'text').css('background-color', '#e6ffcc'); // make it not a date type field and go green
      $(this).click(); // show keyboard
    });
    
    if (vs['usa'] != 1) {
      $('#edit-method-2').click();
      $('#frm-verifyid .form-item-submit a').hide();
    }
    break;

  case 'which':
    var form = fform(fieldId());
    $('#which').modal('show');
    break;

  case 'suggest-who': // called from whoFldSubmit with: field, question, amount, selfErr, restrict, allowNonmember
    suggestWhoScrap();
    break;
    
  case 'new-acct':
    var fid = '#edit-newacct';
    var form = $('#frm-accounts');
    suggestWho(fid, '');
    form.submit(function (e) {
      return who(form, fid, '', false, 'self-switch', '', false); // no question, amount, restriction (and no nonMembers)
    });
    break;

  case 'invest-proposal':
    $('#add-co').click(function () {
      $('.form-item-fullName, .form-item-city, .form-item-zips, .form-item-dob, .form-item-gross, .form-item-bizCats').show();
      require('#edit-fullname, #edit-city, #edit-zips, #edit-dob, #edit-gross, #edit-bizcats', true, true);
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
      showBank($('#edit-connect-0').attr('checked') == 'checked');
      $('#edit-connect-0').click(function () {showBank(true);});
      $('#edit-connect-1').click(function () {showBank(false);});
      $('#edit-connect-2').click(function () {showBank(false);});
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
    suggestWho('#edit-company', ':IS_CO');
    break;
  
  case 'stepup':
    var first = true;
    
    $('[id^="edit-org"]').each(function () {
      if (!$(this).val()) {
        if (first) first = false; else $(this).parents('.row').hide();
      }
    });
    suggestWho('[id^="edit-org"]', ':IS_CO');

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
    suggestWho('#edit-newmember', '');
    break;
    
  case 'rules':
    $('#edit').click(function () {location.href = baseUrl + '/sadmin/rules/id=' + $('#edit-id').val();});
    suggestWho('#edit-payer, #edit-payee, #edit-from, #edit-to', '');
    break;
    
  case 'posts':
    function askAddr() {$('#locset').toggle(); $('#edit-locus').focus();}
    
    $('#menu-signin').hide(); // don't confuse (signin is not required for this feature)

    $('#edit-point').click(function () { // click the Go button
      $('#edit-submitter').click();
      return false; // cancel original link click
    });

    $('#edit-where').click(function () {
      if (vs['noLocYet'] == 1 && vs['isMobile'] == 1 && navigator.geolocation) navigator.geolocation.watchPosition(
        function (z) {location.href = `${baseUrl}/community/posts/latitute=${z.coords.latitude}&longitude=${z.coords.longitude}`;}, 
        function (error) {askAddr();}
      ); else askAddr();
    });

    $('#list .tbody .row').click(function () {location.href = $(this).find('a').attr('href');}); // click any part of box
       
    $('#edit-nogo').click(function () {$('#edit-search').val('').change();});

    $('#edit-search').keydown(function () { // user pressed Enter in search box
      if (event.which == 13) {
        $(this).blur();
        event.preventDefault();
      }
    });

    $('#edit-type, #edit-cat, #edit-terms, #edit-sorg, #edit-search').change(function () {
      var box = $('#list');
      var type = $('#edit-type').find(':selected').val();
      var cat = $('#edit-cat').find(':selected').val();
      var terms = $('#edit-terms').find(':selected').val();
      var sorg = $('#edit-sorg').find(':selected').val();
      var s = $('#edit-search').val().trim().replace(/\s{2,}/g, ' '); // the search string without extraneous spaces
      var cnt;

      var sel = '.tbody .row';
      if (type >= 0) sel += '.t' + type;
      if (cat > 0) sel += (cat == vs['myPosts']) ? '.mine' : ('.c' + cat);
      if (terms >= 0) sel += '.x' + terms;
      if (sorg >= 0) sel += '.s' + sorg;
      box.find('.tbody .row').hide(); // hide all
      box = box.find(sel); // initial selection before search
      cnt = box.show().length; // show everything in the chosen category (and count them)
    
      if (s.length) { // searching, so narrow the selection
        var i, words = s.toUpperCase().split(' '); // array of words
        var cols = '.cat .item .details'.split(' ');

        box.each(function () { // eliminate non-matches
          colText = ''; for (i in cols) colText += ' ' + $(this).find(cols[i]).text().toUpperCase(); // the item's text
          for (i in words) if (colText.indexOf(words[i]) < 0) { // does it fail to match any search word?
            $(this).hide(); // show only if it has all words
            cnt -= 1;
          }
        });
      }
      $('#none').toggle(cnt <= 0); // and the "nothing found in this area" row, if any
    });
     
    
    $('#edit-view').click(function () {
      if ($('#list.memo').length > 0) { // if memo view, switch to list view
        $('#list').removeClass('memo');
        $(this).text(vs['memoView']);
      } else { // list view, switch to memo
        $('#list').addClass('memo');
        $(this).text(vs['listView']);
      }
    });
    break;
    
  case 'post-post':
//    $('#edit-cat').change(function () {setCookie(vs['type'] + 'cat', $(this).val());});
    $('input[name="type"]').change(function () {
      var type = vs['types'].split(' ')[$(this).val()];
      var need = (type == 'need');
      $('.form-item-service').toggle(type != 'tip');
      $('.form-item-radius').toggle(!need); 
//      $('.form-item-exchange').toggle(need);
      if ($('#edit-radius').val() == '') $('#edit-radius').val(type == 'tip' ? 0 : 10); // tips default to everywhere
    });
    $('.form-item-end a').click(function () {
      $('#edit-end').attr('type', 'text').val(new Date(Date.now()).toLocaleString('en-US', {month:'2-digit', day:'2-digit', year:'numeric'}).split(',')[0]);
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

  case 'signupco':
    $('#edit-agentqid').keyup(function () {reqQ($('.form-item-pass'), $('#edit-agentqid').val().trim() != '');});
    break;
    
  case 'agree':
    $('#show-agreement').click(function () {$('#wrap-agreement').toggle();}); 
    break;
    
  case 'prejoint': $('#edit-old-0').click(function () {this.form.submit();}); break;

  case 'invite-link': $('#inviteLink').click(function () {SelectText(this.id);}); break;
  
  case 'invite':
    function showInviteFlds() {
      $('.form-item-question, .form-item-quote, .form-item-org, #edit-usePhoto, #edit-postPhoto').show();
    }
    $('#edit-sign-0').click(function () {$('.form-item-whyNot').show();});
    $('#edit-sign-1').click(function () {showInviteFlds(); $('.form-item-whyNot').hide();});
    $('#edit-org').keyup(function () {
      if ($(this).val().length > 0) $('.form-item-position, .form-item-website').show();
    });
    if (vs['edit'] == 1) {showInviteFlds(); $('#edit-org').keyup();}
    break;

  case 'shouters':
    $('.rating').click(function () {
      post('bumpShout', {qid:$(this).attr('qid')}, null);
      var rating = $(this).attr('rating');
      $(this).attr('rating', rating == '3' ? 0 : (parseInt(rating) + 1));
    });
    break;
    
  case 'amtChoice':
    var amtChoiceWrap = $('.form-item-amtChoice');
    var amtChoice = $('#edit-amtchoice');
    var other = $('.form-item-amount'); 

    amtChoice.change(function () {
      if(amtChoice.val() == '-1') {
        other.show(); 
        amtChoiceWrap.hide();
        $('#edit-amount').focus();
      } else other.hide();
    });
    amtChoice.change();
    
    $('#edit-amount').change(function () {if ($(this).val().trim() == '0') $('#edit-often').val('Y');});
    break;
    
  case 'contact':
    var form = $('#frm-contact');
    $('#edit-fullname', form).focus();
    $('#edit-email', form).change(function () {$('.form-item-pass').show(); $('#edit-pass').focus();});
    form.submit(function (e) {return setPostalAddr(false);});
    break;

  case 'verifyemail':
    var pw = $('.form-item-pw');
    if (vs['verify'] == 1) {
      reqNot(pw);
    } else pw.show().focus();
    $('.form-item-showPass input').click(function() {
      pw.toggle();
      if (pw.is(':visible')) $('#edit-pw').focus();
    });
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
    
  case 'notice-prefs':
    $('.k-freq select').change(function () {
      post('setNotice', {code:vs['code'], type:$(this).attr('name').replace('freq-', ''), freq:$(this).val()}, reportErr);
    });
    break;
        
  default:
    alert('ERROR: Unknown script scrap (there is no default script).');
    alert($('#script-scraps').attr('src').replace(/^[^\?]+\??/,''));
    
  }
}

/**
 * Show or hide ID verification fields according to verification method
 */
function verifyid(method) {
  $('#edit-method-' + method).prop('checked', true); // needed after error
  var ssn = $('.form-item-federalId');
  var dob = $('.form-item-dob');
  var idtype = $('.form-item-idtype');
  var file = $('.form-item-file');

  req(dob);
  if (method == 0) {req(ssn); reqNot(file); reqNot(idtype); $('#edit-federalid').focus();}
  if (method == 1) {reqNot(ssn); req(file); reqNot(idtype);}
  if (method == 2) {reqNot(ssn); req(file); req(idtype); $('#edit-idtype').focus();}
}  

function setInvestFields() {
  var equity = ($('input[name="equity"]:checked').val() == 1);
  $('.form-item-equitySet').toggle(equity);
  $('.form-item-loanSet').toggle(!equity);
  require('#edit-offering, #edit-price, #edit-return', equity, true);
  require('#edit-offering--2, #edit-price--2, #edit-return--2', !equity, true);
}

/**
 * Return the log of the entropy of a given password
 * No. For now, return 1 point for each: UTF8, upper, lower, digit, punc, 7 chars
 */
function pwScore(s) {
  var e = u(s, 'U') + u(s, 'A') + u(s, 'a') + u(s, 'n') + u(s, 'p'); // character universe size (0-224)
//  return s.length * Math.log(Math.max(1, e));
  return e + s.length / 7;

  /**
   * Return the size of the character universe (U=unicode, A=cap, a=lower, n=digit, p=punc)
   * But utf8mb4 characters are 75% discounted, because emojis make them easy to select.
   * Actually no. Just return 1 for a match and 0 otherwise (for now).
   */
  function u(s, u) {
    var us = {U:'[^\u0000-\u007f]+', A:'[A-Z]+', a:'[a-z]+', n:'[0-9]+', p:'[^[A-Za-z0-9]+'};
    var score = {U:(256-32)/4, A:26, a:26, n:10, p:128-26-26-10-32};
//    return (new RegExp(us[u]).test(s) ? score[u] : 1);
    return (new RegExp(us[u]).test(s) ? 1 : 0);
  }
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

function cardOther(focus) {
  $('.form-item-for').hide();
  $('#edit-desc').val('').attr('required', 'required');
  $('.form-item-desc').show();
  if (focus) $('#edit-desc').focus();
}

function hideComment() {
  var note = $('.form-item-comment');
  note.hide();
  $('.form-item-submit a').click(function () {$('.form-item-submit a').hide(); note.show().find('textarea').focus();});
}

function req(fld) {fld.show(); fld.find('input').attr('required', 'required');}
function reqNot(fld) {fld.find('input').removeAttr('required'); fld.hide();}
function reqQ(fld, show, optional) {
  if (optional) {
    fld.toggle(show);
  } else {
    if(show) req(fld); else reqNot(fld);
  }
}

function confirmTip(vs, val) {
  var tipDol = has(val, '%') ? '' : '$';
  var total = '$' + fmtAmt(parseFloat(vs['amt']) + parseFloat(val) * (tipDol ? 1 : (parseFloat(vs['amt']) / 100)));
  var title = vs['title'].replace('%tip', tipDol ? tipDol + fmtAmt(val) : val);
  confirm(title, vs['msg'].replace('%total', total), function () {
    location.href = baseUrl + '/card/tip/xid=' + vs['xid'] + '&tip=' + val.replace('%', '!'); // exclamation point means percent in URL
  });
  return false;
}
