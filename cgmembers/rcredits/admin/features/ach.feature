Feature: Checks and Deposits
AS an administrator
I WANT to print checks from members to rCredits
SO we can accept their US Dollars in exchange for rCredits

AND I WANT to print checks from rCredits to members
So we can accommodate members' requests to cash out

AND I WANT to display deposit statements and individual checks for current and past deposits
SO I can make deposits easily and review past deposits as necessary

Setup:
  Given members:
  | uid  | fullName | floor | flags              | postalAddr          | phone | bankAccount     | coFlags   |*
  | .ZZA | Abe One  | -500  | ok,confirmed,admin | 1 A, Aton, MA 01001 |     1 | USkk21187028101 |           |
  | .ZZB | Bea Two  | -500  | ok,confirmed,co    | 2 B, Bton, MA 01002 |     2 | USkk21187028102 |           |
  | .ZZC | Cor Pub  |    0  | ok,confirmed,co    | 3 C, Cton, MA 01003 |     3 | USkk21187028103 | sponsored |
  And these "admins":
  | uid  | vKeyE     | can                     |*
  | .ZZA | DEV_VKEYE | v,seeDeposits,ach,panel |
  And these "people":
  | pid | fullName |*
  |   6 | Flo Six  |
  
Scenario: admin downloads ACH requests
  Given these "txs2":
  | txid | payee | amount | created | deposit  | completed | pid | bankAccount     |*
  | 5001 | .ZZA  |    100 | %now-3w | %now0-2w | %now-3w   |     | USkk21187028101 |
  | 5002 | .ZZA  |    400 | %now-2w |        0 | %now      |     | USkk21187028101 |
  | 5003 | .ZZB  |   -100 | %now-1d |        0 | %now      |     | USkk21187028102 |
  | 5004 | .ZZC  |    300 | %now    |        0 |         0 |     | USkk21187028103 |
  | 5005 | .ZZC  |     60 | %now    |        0 |         0 |   6 | USkk21187028106 |
  And member ".ZZA" is signed in
  And member ".ZZA" scans admin card "DEV_VKEYPW"

  When member ".ZZA" visits page "sadmin/deposits"
  Then we show "Bank Transfers" with:
  | New IN    | 3     | $760  |
  | New OUT   | 1     | $-100   |
  And with:
  |            | Transfers |  Count | Total     |
  | TODAY      | IN        |      3 | 760.00    |
  |            | OUT       |      1 | -100.00   |
  | %mdY-2w    | IN        |      1 | 100.00    |

  When member ".ZZA" visits page "sadmin/achs/date=0&mark=1"
  Then we download "BSL-<NOW>.ach" with "ACH" records:
  
# const F_FILEHDR = 'recType:R1, priority:R2, sp:L1, destNum:R9, originPrefix:L1, originNum:R9, datetime:R10, fileIdModifier:R1, recSize:R3, blocking:R2, format:R1, destName:L23, originName:L23, ref:L8';
# const F_BATCHHDR = 'recType:R1, class:R3, originName:L16, data:L20, companyId:R10, secCode:L3, purpose:L10, when:L6, entryDate:R6, settle:L3, status:R1, destNum:R8, batchNum:R7';
# const F_PPD = 'recType:R1, txCode:R2, routing:R9, account:L17, amount:R10, id:L15, name:L22, data:L2, addendaFlag:R1, destNum:R8, count:R7'; // Prearranged Payment and Deposit (entry detail)
# const F_BATCHFTR = 'recType:R1, class:R3, count:R6, hash:R10, debits:R12, credits:R12, companyId:R10, auth:L19, reserve:L6, destNum:R8, batchNum:R7';
# const F_FILEFTR = 'recType:R1, batches:R6, blocks:R6, entries:R8, hash:R10, debits:R12, credits:R12, reserve:L39';

  | 1,01, ,BANKROUTE, ,%CGF_EIN,DATETIME,0,094,10,1,BANK,%PROJECT, |
  | 5,200,%PROJECT,,,1%CGF_EIN,PPD,CG Credit ,WHEN,,,1,BANKROUT,0000001 |
  | 6,22,BA-B,100,NEWZZB,Bea Two,0,BANKROUT,0000001 |
  | 6,27,BA-A,400,NEWZZA,Abe One,0,BANKROUT,0000002 |
  | 6,27,BA-C,300,NEWZZC,Cor Pub,0,BANKROUT,0000003 |
  | 6,27,21187028106,60,6,Flo Six,0,BANKROUT,0000004 |
  | 8,200,4,,760,100,,1%CGF_EIN,,,BANKROUT,0000001 |
  | 9,000001,000001,00000004,,760,100, |
  
  And these "txs2":
  | txid | payee | amount | created | deposit  | completed |*
  | 5001 | .ZZA  |    100 | %now-3w | %now0-2w | %now-3w   |
  | 5002 | .ZZA  |    400 | %now-2w | %now     | %now      |
  | 5003 | .ZZB  |   -100 | %now-1d | %now     | %now      |  
  | 5004 | .ZZC  |    300 | %now    | %now     |         0 |
  | 5005 | .ZZC  |     60 | %now    | %now     |         0 |
