var getv = parseUrlQuery($('#script-new-acct').attr('src').replace(/^[^\?]+\??/,''));
//alert($('#script-charts').attr('src').replace(/^[^\?]+\??/,''));

var fid = '#edit-newacct';
var form = $('#frm-accounts');

var members = new Bloodhound({
//  datumTokenizer: Bloodhound.tokenizers.obj.whitespace('value'),
  datumTokenizer: Bloodhound.tokenizers.whitespace,
  queryTokenizer: Bloodhound.tokenizers.whitespace,
  prefetch: {
    url: ajaxUrl + '?op=typeWho&sid=' + ajaxSid,
    cache: false
  }
});
$(fid).wrap('<div></div>').typeahead(
  {
    minLength: 3,
    highlight: true
  },
  {
    name: 'acctChoices',
//    display: 'value',
    source: members
  }
);

form.submit(function (e) {
  if ($(fid).val() == '') return true; // don't show the "which" form twice
  return who(form, fid, getv.question, false, true, false);
});

