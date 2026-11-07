
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

  When member ".ZZA" scans admin card "DEV_VKEYPW"
  Then cryptcookie "vKeyPw" decrypted is "DEV_VKEYPW"

  When member ".ZZA" visits "sadmin/checks/way=In&date=%yesterday"
  Then we show PDF with:
  | Bea Two | 123.00 | for %PROJECT credit |

  Given member ".ZZB" is signed in
  When member ".ZZB" scans admin card "DEV_VKEYPW"
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

Scenario: The PHP window is for local machines and for admins with the code permission
  Given these "admins":
  | uid  | vKeyE     | can  |*
  | .ZZA | DEV_VKEYE | code |
  Then the PHP window, away from a local machine, is open to:
  | who   | open |*
  | .ZZA  | yes  |
  | .ZZB  | no   |
  | super | no   |

Scenario: An administrator password must score higher than a member's
  Then these passwords are strong enough for an admin:
  | pw                           | avoid  | ok  |*
  | k                            |        | no  |
  | abcdefghijk                  |        | no  |
  | zzzzzzzzzzzzzzz1             |        | no  |
  | Password1234!                |        | no  |
  | qwertyuiop12                 |        | no  |
  | aB3$efghi                    |        | no  |
  | aB3$efghij                   |        | yes |
  | aB3$efghij                   | efghij | no  |
  | tnmkrhzjwpqlxyuP1           |        | yes |
  | correct horse battery staple |        | yes |

Scenario: An admin with a weak password cannot sign in to production
  Then admin sign-in with these passwords is refused on production:
  | who  | pw           | refused |*
  | .ZZA | k            | yes     |
  | .ZZA | aB3$efghij   | no      |
  | .ZZB | k            | no      |
