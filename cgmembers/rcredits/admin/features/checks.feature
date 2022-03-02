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
  | uid  | fullName | floor | flags              | postalAddr          | phone | bankAccount     |*
  | .ZZA | Abe One  | -500  | ok,confirmed,admin | 1 A, Aton, MA 01001 |     1 | USkk21187028101 |
  | .ZZB | Bea Two  | -500  | ok,confirmed,co    | 2 B, Bton, MA 01002 |     2 | USkk21187028102 |
  | .ZZC | Cor Pub  |    0  | ok,confirmed,co    | 3 C, Cton, MA 01003 |     3 | USkk21187028103 |
  And these "admins":
  | uid  | vKeyE     | can                             |*
  | .ZZA | DEV_VKEYE | v,seeDeposits,printChecks,panel |
  
Scenario: admin prints checks
  Given these "txs2":
  | txid | payee | amount | created   | deposit      | completed |*
  | 5001 | .ZZA  |    100 | %today-3w | %daystart-2w | %today-3w |
  | 5002 | .ZZA  |    400 | %today-2w |            0 | %today    |
  | 5003 | .ZZB  |    100 | %today-1d |            0 | %today    |  
  | 5004 | .ZZC  |    300 | %today    |            0 |         0 |
  And member ".ZZA" scans admin card "%DEV_VKEYPW"
  When member ".ZZA" visits page "sadmin/deposits"
  Then we show "Bank Transfers" with:
  | New IN | 3 |

  When member ".ZZA" visits page "sadmin/checks/way=IN&date=0&mark=1"
  Then we show PDF with:
  |~name    |~postalAddr          |~phone        |~transit      |~acct |~xid |~date |~amt   |~amount |~bank |*
  | Abe One | 1 A, Aton, MA 01001 | 413 772 0001 | 53-7028/2118 |   01 |   2 | %mdY | $ 400 | Four Hundred and NO/100 | Greenfield Co-op Bank |
  | Bea Two | 2 B, Bton, MA 01002 | 413 772 0002 | 53-7028/2118 |   02 |   3 | %mdY | $ 100 | One Hundred and NO/100 | Greenfield Co-op Bank |
  | Cor Pub | 3 C, Cton, MA 01003 | 413 772 0003 | 53-7028/2118 |   03 |   4 | %mdY | $ 300 | Three Hundred and NO/100 | Greenfield Co-op Bank |  
  And these "txs2":
  | txid | deposit   | xid |*
  | 5002 | %daystart |   2 |
  | 5003 | %daystart |   3 |
  | 5004 | %daystart |   4 |
  And balances:
  | uid  | balance |*
  | .ZZA |     500 |
  | .ZZB |     100 |
  | .ZZC |       0 |
