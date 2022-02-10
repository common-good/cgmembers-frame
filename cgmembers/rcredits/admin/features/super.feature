Feature: Super
AS a SuperAdministrator
I WANT to do super stuff that normal admins can't do
SO I can handle routine operations of the regional server, such as bank transfers and card-printing

Setup:
  Given members:
  | uid  | fullName | flags               |*
  | .ZZA | Abe One  | ok,confirmed,admin  |
  | .ZZB | Bea Two  | ok,confirmed        |
  | .ZZC | Cor Pub  | ok,confirmed,co     |
  
Scenario: An admin prints checks
  Given these "admins":
  | uid  | vKeyE     | can                             |*
  | .ZZA | DEV_VKEYE | v,seeDeposits,printChecks,panel |
  And these "txs2":
  | xid  | payee | amount | deposit    |*
  | 3    | .ZZB  | 123    | %yesterday |
  And member ".ZZA" is signed in

  When member ".ZZA" scans admin card "%DEV_VKEYPW"
  Then cryptcookie "vKeyPw" is "%DEV_VKEYPW" decrypted

  When member ".ZZA" visits "sadmin/checks/way=In&date=%yesterday"
  Then we show PDF with:
  | Bea Two | 123.00 | for %PROJECT credit |

  Given member ".ZZB" is signed in
  When member ".ZZB" scans admin card "%DEV_VKEYPW"
  Then we say "error": "no page permission" with:
  | page | Panel |**

Scenario: A member tries to do an admin thing
  When member ".ZZB" visits "sadmin/checks/way=In&date=%yesterday"
  Then we say "error": "no page permission" with:
  | page | Checks |**

Scenario: An admin tries to do an admin thing with insufficient permissions
  When member ".ZZA" visits "sadmin/checks/way=In&date=%yesterday"
  Then we say "error": "no page permission" with:
  | page | Checks |**

Scenario: A superAdmin grants super permission to another admin
