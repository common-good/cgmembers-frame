<?php
namespace CG\Web;
use CG\Web as w;
use CG as r;
use CG\Backend as be;
use CG\Util as u;
use CG\Db as db;

const MAX_OFAC_SIZE = 10 * 1024 * 1024;
const OFAC_FILE_CNT = 4;

/**
 * Upload invoices spreadsheet to create invoices.
 * The validate and submit functions are also called by PaymentsUpload and Payroll.
 */
function formOFAC($form, &$sta) {
  global $mya;

  $title = t('Upload OFAC Files');
  $subtext = t(
   '<p>Processing this upload might take 5 minutes or so, so be patient.</p>
    <p>The Office of Foreign Assets Control (OFAC) provides names of known foreign criminals and their aliases (ALT names). We must not allow them to use %PROJECT. Upload new files here every month.</p>
    <p>Here are the direct download links for CSV files from the <%aSdnPage>SDN page</a> and the <%aConsPage>Consolidated List page</a>:</p>
    <ul>
      <li><%aSdn>sdn.csv</a></li>
      <li><%aSdnAlt>alt.csv</a></li>
      <li><%aCons>cons_prim.csv</a></li>
      <li><%aConsAlt>cons_alt.csv</a></li>
    </ul>',

    '_aSdnPage _aConsPage _aSdn _aSdnAlt _aCons _aConsAlt', 

      atag('https://sanctionslist.ofac.treas.gov/Home/SdnList'), 
      atag('https://sanctionslist.ofac.treas.gov/Home/ConsolidatedList'),
      atag('https://sanctionslistservice.ofac.treas.gov/api/PublicationPreview/exports/SDN.XML'),
      atag('https://sanctionslistservice.ofac.treas.gov/api/PublicationPreview/exports/ALT.CSV'),
      atag('https://sanctionslistservice.ofac.treas.gov/api/PublicationPreview/exports/CONS_PRIM.CSV'),
      atag('https://sanctionslistservice.ofac.treas.gov/api/PublicationPreview/exports/CONS_ALT.CSV')
  );
  $file1 = fileFld(t('SDN file:'));
  $file2 = fileFld(t('ALT file:'));
  $file3 = fileFld(t('CONS file:'));
  $file4 = fileFld(t('CONS ALT file:'));
  $submit = t('Upload');

  return cgform(compact(ray('title subtext file1 file2 file3 file4 submit')));
}

function formOFAC_validate($form, &$sta) {
  for ($i = 1; $i <= OFAC_FILE_CNT; $i++) {
    $data = uploadFile(NULL, 'text/csv', "file$i", ray('maxSize', MAX_OFAC_SIZE));
    if (!is_array($data)) return err("$data (file$i)", "file$i");
    $sta["path$i"] = $data['path'];
  }
}

function formOFAC_submit($form, &$sta, $args = NULL) {

  db\q('TRUNCATE ofac');
  $cnt = 0;

  for ($i = 1; $i <= OFAC_FILE_CNT; $i++) {
    if (!$f = fopen($sta["path$i"], "r")) return u\FAIL(t('Can\'t open file #%i', compact('i')));
    while ($row = fgetcsv($f) and count($row) >= 3 and ++$cnt) { // count($row) >= 3 needed because last line+1 returns just a weird character (EOF?) in $row[0]
      $nm = $row[u\isOdd($i) ? 1 : 3];
      $co = preg_match('[a-z]', $nm) ?: 0; // companies are all caps, individuals are mixed-case (alt files don't specify which are companies)
      if (!$co and strhas($nm, ',')) { // put last name last
        $nmRay = explode(',', $nm, 2);
        $nm = trim($nmRay[1] . ' ' . $nmRay[0]);
      }
      $nm = strtolower($nm);
      db\insert('ofac', compact(ray('nm co')), 'nm');
    }
    fclose($f);
    u\deleteFile($sta["path$i"]);
  }
  setv('ofac_updated', now());
  say(t('Recreated OFAC file with %cnt entries.', compact('cnt')));
}
