var statesStr = '' // must match r_states table in database
+ 'AL:Alabama,'
+ 'AK:Alaska,'
+ 'AZ:Arizona,'
+ 'AR:Arkansas,'
+ 'CA:California,'
+ 'CO:Colorado,'
+ 'CT:Connecticut,'
+ 'DE:Delaware,'
+ 'FL:Florida,'
+ 'GA:Georgia,'
+ 'HI:Hawaii,'
+ 'ID:Idaho,'
+ 'IL:Illinois,'
+ 'IN:Indiana,'
+ 'IA:Iowa,'
+ 'KS:Kansas,'
+ 'KY:Kentucky,'
+ 'LA:Louisiana,'
+ 'ME:Maine,'
+ 'MD:Maryland,'
+ 'MA:Massachusetts,'
+ 'MI:Michigan,'
+ 'MN:Minnesota,'
+ 'MS:Mississippi,'
+ 'MO:Missouri,'
+ 'MT:Montana,'
+ 'NB:Nebraska,'
+ 'NV:Nevada,'
+ 'NH:New Hampshire,'
+ 'NJ:New Jersey,'
+ 'NM:New Mexico,'
+ 'NY:New York,'
+ 'NC:North Carolina,'
+ 'ND:North Dakota,'
+ 'OH:Ohio,'
+ 'OK:Oklahoma,'
+ 'OR:Oregon,'
+ 'PA:Pennsylvania,'
+ 'RI:Rhode Island,'
+ 'SC:South Carolina,'
+ 'SD:South Dakota,'
+ 'TN:Tennessee,'
+ 'TX:Texas,'
+ 'UT:Utah,'
+ 'VT:Vermont,'
+ 'VA:Virginia,'
+ 'WA:Washington,'
+ 'WV:West Virginia,'
+ 'WI:Wisconsin,'
+ 'WY:Wyoming,'
+ 'DC:District of Columbia,'
+ ':,'
+ 'AS:American Samoa,'
+ 'GU:Guam,'
+ ':,'
+ 'MP:Northern Mariana Islands,'
+ 'PR:Puerto Rico,'
+ 'VI:Virgin Islands,'
+ 'UM:United States Minor Outlying Islands,'
+ 'AE:Armed Forces Europe,'
+ 'AA:Armed Forces Americas,'
+ 'AP:Armed Forces Pacific'
;

sts = new Array();
states = new Array();
i = 1000;
for (state of statesStr.split(',')) {
  state = state.split(':');
  sts[i] = state[0];
  states[i] = state[1];
  i++;
}

$ = jQuery;

/**
 * On click of "sameAddr" field, set physical address fields equal to postal address fields.
 * On submit of signup or contact form, set (aggregate) postalAddr field and return true.
 */
function setPostalAddr(same) {
  if (same) {
    $('.form-item-sameAddr').hide();
    var fields = ['address', 'city', 'state', 'zip'];
    for (i = 0; i < fields.length; i++) $('#edit-' + fields[i]).val($('#edit-' + fields[i] + '2').val());
  } else {
    var state = $('#edit-state2').val();
    var city = $('#edit-city2').val();
    var address = $('#edit-address2').val();
    var zip = $('#edit-zip2').val();
    var postalAddr = $('#edit-postaladdr');

    postalAddr.val(address + ', ' + city + ', ' + sts[state] + ' ' + zip);
    return true;
  }
}

function zipChange(z3s) {
//    $.zipLookupSettings.libDirPath = 'http://ziplookup.googlecode.com/git/public/ziplookup/'; 
    $.zipLookupSettings.libDirPath = '../rcredits/inc/'; 
    $.zipLookup(
      $('#edit-zip').val(),
      function(cityName, stateName, stateShortName){ // success
        $('#edit-city').val(cityName);
        $('#edit-state').val(sts.indexOf(stateShortName));
//        $('.message').html("Found Zipcode");
      },
      function(errMsg){ // zip couldn't be found,
//    alert(errMsg);
//          $('.message').html("Error: " + errMsg);
      }
    );

}
