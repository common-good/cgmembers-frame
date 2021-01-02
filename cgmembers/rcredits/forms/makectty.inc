<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

/**
 * Create a new community or region.
 */
function formMakeCtty($form, &$sta) {
  $title = t('Make a New Community');
  $subtext = t('<p>Typically the name of a %PROJECT community is "%PROJECT" followed by the place name; for example, "<b>%PROJECT Greenfield MA</b>". The legal name for a new organization should include the state (for disambiguation), spelled out; for example, "%PROJECT Greenfield Massachusetts".</p><p>Email should reflect the legal name, for example "<b>greenfieldMA@%CG_DOMAIN</b>".</p>');
  $region = textFld(t('Region:'), [t('Region code'), t('3-letter code of community\'s region')], required());
  $zips = textFld(t('Zip Regex:'), [t('Zipcodes to include'), t('|Type a regular expression for all zipcodes to be included (by default) in the community. For example, ^013|01002 would include all zipcodes starting 013 plus zipcode 01002. Individual members can also be moved manually in or out of this community.')]);

  // from w\contactFields
  $phone = phoneFld(REQ . t('Phone:'), [t('Contact phone number')]);
  $postalAddr = textFld(REQ . t('Postal Addr:'), [t('Complete mailing address'), t('Where does the post office send your mail (usually)?')], dft(nn($postalAddr)));

  $submit = submit();
  
  $form = compact('title', 'subtext') + nameAndEmailFlds(-1) + ssnFields(FALSE)
    + compact(ray('phone postalAddr region zips submit'));
  return cgform($form);
}

function formMakeCtty_validate($form, &$sta) {
  global $mya;
  $myid = $mya->id;
  extract(u\normalizeCase(just($normals = 'fullName legalName postalAddr', $sta['input'], NULL)));
  extract(just('zips region email phone federalId', $sta['input'], NULL));

  if (preg_match("~$zips~", null) === FALSE) return say(t('bad Zips REGEX'), 'zips');
  if (!$region or !preg_match('/[A-Z]{3}/i', $region)) return say(t('bad region code'), 'region');

  if (!$fullName) $fullName = $legalName;
  foreach (ray('fullName legalName') as $field) {
    if ($err = u\badName($$field)) return say($err, compact('field'), $field);
  }
  if (!emailOkay($email, $mya->co, $mya)) return; 
  if ($err = u\badPhone($phone)) return say($err, 'phone');
  $name = u\shortName($fullName);
  if ($otherName = db\get('fullName', 'users', 'name=:name', compact('name'))) return say('shortname taken', compact('otherName'), 'fullName');

  if ($err = u\badSsn($federalId)) return say($err, ['what' => 'federalId'], 'federalId');
  
  list ($address, $city, $state, $zip) = u\parseAddr($postalAddr);
  if (!$address) return say('Bad postal address', 'postalAddr');
  if (!$state = r\stateNum($state)) return say('Bad state', 'postalAddr');
  if ($err = u\badZip($zip)) return say($err, 'postalAddr');

  $region = strtoupper($region);
  if (!db\exists('r_regions', 'region=:region', compact('region'))) return say(t('That region does not exist.'), 'region');

  u\preray(compact(ray($normals . ' name email phone address city state zip region federalId')), $sta['input']); // fix formatting
}

function formMakeCtty_submit($form, &$sta) {
  $fields = 'name fullName legalName email phone federalId postalAddr address city state zip';
  extract(just('zips region ' . $fields, $sta['input']));

  $flags = u\bit(B_CO) | u\bit(B_OK) | u\bit(B_UP);
  $country = R_COUNTRY_ID;
  $region = r\qo("!$region")->id; // negative region tells newAcct to create a community instead of a regular account
  $coType = CO_PARTNERSHIP;
  $fields .= ' flags region address state zip country coType';

  $DBTX = \db_transaction();
  $info = compact(ray($fields));
  
  if (!$a = new r\Acct($info)) return say('cannot create account', 'zot');
  $a->update('community', $a->id);

  db\update('r_company', ray('uid zips', $a->id, $zips), 'uid');
  unset($DBTX); // commit
  say('info saved');
}
