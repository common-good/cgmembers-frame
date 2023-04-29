Feature: Accounts
AS a member
I WANT to install the CGPay app on my device
SO I can use it to charge customers through the Common Good system (or eventually to pay them or access other Common Good system features)

Setup:
  Given members:
  | uid  | fullName | phone  | email | cc  | cc2  | flags  | pass | city | state |*
  | .ZPA | Abe One  | +20001 | a@    | ccA | ccA2 | ok     | Aa1  | Aton | AL    |
  | .ZPB | Bea Two  | +20002 | b@    | ccB |      | ok     |      | Bton | MA    |
  | .ZPC | Coco Co  | +20003 | c@    | ccC |      | ok,co  |      | Cton | CA    |
  | .ZPF | For Co   | +20006 | f@    | ccF |      | co     |      | Fton | FL    |
  And these "u_company":
  | uid  | selling |*
  | .ZPC | bags    |
  | .ZPF | stuff   |
  And these "r_boxes":
  | uid  | code |*
  | .ZPC | devC |
  And these "u_relations":
  | main | agent | num | permission |*
  | .ZPC | .ZPA  |   1 | manage     |
  | .ZPC | .ZPB  |   2 | manage     |
  | .ZPF | .ZPA  |   1 | manage     |
  And member ".ZPA" has "card" steps done: "all"

# GET /accounts

Scenario: A member signs in to the app for a list of accounts to choose from
  Given next random code is "whatever"
  And var "accounts" is JSON:
  | accountId | deviceId | qr   | isCo  | name    | selling |*
  | K6VMDCA   | whatever | ?    | false | Abe One | %NUL    |
  | L6VMDCC0  | whatever | %NUL | true  | Coco Co | [bags]  |
  When app posts "accounts" with:
  | identifier | password |*
  | .ZPA       | Aa1      |
  Then we reply "ok" with JSON:
  | accounts | %accounts |**

Scenario: A member tries to sign in without an identifier
  When app posts "accounts" with:
  | identifier | password |*
  |            | Aa1      |
  Then we reply "unauth" with: "?"

Scenario: A member tries to sign in without a password
  When app posts "accounts" with:
  | identifier | password |*
  | .ZPA       |          |
  Then we reply "unauth" with: "?"

Scenario: A member tries to sign in without a non-existent account identifier
  When app posts "accounts" with:
  | identifier | password |*
  | .ZPZ       | Aa1      |
  Then we reply "notfound" with: "?"

Scenario: A member tries to sign in without a totally bad identifier
  When app posts "accounts" with:
  | identifier | password |*
  | piffle     | Aa1      |
  Then we reply "notfound" with: "?"

Scenario: A member tries to sign in without a bad password
  When app posts "accounts" with:
  | identifier | password |*
  | .ZPA       | zork     |
  Then we reply "notfound" with: "?"
