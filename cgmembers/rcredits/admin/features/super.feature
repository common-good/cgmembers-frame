Feature: Super
AS a SuperAdministrator
I WANT to do super stuff that normal admins can't do
SO I can handle routine operations of the regional server, such as bank transfers and card-printing

Setup:
  Given members:
  | uid  | fullName | flags              |*
  | .ZZA | Abe One  | ok,confirmed,admin |
  | .ZZB | Bea Two  | ok,confirmed       |
  | .ZZC | Cor Pub  | ok,confirmed,co    |
  
Scenario: A superAdmin grants super permission to another admin
  Given these "admins":
  | uid  | vKeyE     | can   |*
  | .ZZA | DEV_VKEYE | super |
  And member ".ZZA" is signed in
  When member ".ZZA" scans admin card "%DEV_VKEYPW"
  Then cryptcookie "vKeyPw" is "%DEV_VKEYPW"
  
  When member ".ZZA" visits "sadmin/deposits"
  Then we show "Bank Transfers"
  
  When member ".ZZB" is signed in
  And member ".ZZB" scans admin card "%DEV_VKEYPW"
  And member ".ZZB" visits "sadmin/deposits"
  Then we show ""
