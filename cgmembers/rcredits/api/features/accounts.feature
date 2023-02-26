Feature: Accounts
AS a member
I WANT to install the CGPay app on my device
SO I can use it to charge customers through the Common Good system (or eventually to pay them or access other Common Good system features)

Setup:
  Given members:
  | uid  | fullName | phone  | email | cc  | cc2  | flags  | pass | city | state |*
  | .ZZA | Abe One  | +20001 | a@    | ccA | ccA2 | ok     | Aa1  | Aton | AL    |
  | .ZZB | Bea Two  | +20002 | b@    | ccB |      | ok     |      | Bton | MA    |
  | .ZZC | Coco Co  | +20003 | c@    | ccC |      | ok,co  |      | Cton | CA    |
  | .ZZF | For Co   | +20006 | f@    | ccF |      | co     |      | Fton | FL    |
  And these "u_company":
  | uid  | selling |*
  | .ZZC | bags    |
  | .ZZF | stuff   |
  And these "r_boxes":
  | uid  | code |*
  | .ZZC | devC |
  And these "u_relations":
  | main | agent | num | permission |*
  | .ZZC | .ZZA  |   1 | manage     |
  | .ZZC | .ZZB  |   2 | manage     |
  | .ZZF | .ZZA  |   1 | manage     |
  And member ".ZZA" has "card" steps done: "all"

# GET /accounts

Scenario: A member signs in to the app for a list of accounts to choose from
  Given next random code is "whatever"
  And var "accounts" is JSON:
  | accountId | deviceId | qr   | isCo  | name    | selling |*
  | K6VMDJI   | whatever | ?    | false | Abe One | []      |
  | K6VMDJK   | whatever | %NUL | true  | Coco Co | [bags] |
  When app gets "accounts" with:
  | identifier | password |*
  | .ZZA       | Aa1      |
  Then we reply "got" with JSON:
  | accounts | %accounts |**

Scenario: A member tries to sign in without an identifier
  When app gets "accounts" with:
  | identifier | password |*
  |            | Aa1      |
  Then we reply "unauth" with: "?"

Scenario: A member tries to sign in without a password
  When app gets "accounts" with:
  | identifier | password |*
  | .ZZA       |          |
  Then we reply "unauth" with: "?"

Scenario: A member tries to sign in without a non-existent account identifier
  When app gets "accounts" with:
  | identifier | password |*
  | .ZZZ       | Aa1      |
  Then we reply "notfound" with: "?"

Scenario: A member tries to sign in without a totally bad identifier
  When app gets "accounts" with:
  | identifier | password |*
  | piffle     | Aa1      |
  Then we reply "notfound" with: "?"

Scenario: A member tries to sign in without a bad password
  When app gets "accounts" with:
  | identifier | password |*
  | .ZZA       | zork     |
  Then we reply "notfound" with: "?"
