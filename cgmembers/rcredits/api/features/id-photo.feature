Feature: ID Photo
AS a member
I WANT to securely identify a customer who presents their CG card for payment
SO I can subsequently charge them

Setup:
  Given members:
  | uid  | fullName | phone  | email | cc  | cc2  | flags  | pass | city | state |*
  | .ZZA | Abe One  | +20001 | a@    | ccA | ccA2 | ok     | Aa1  | Aton | AL    |
  | .ZZB | Bea Two  | +20002 | b@    | ccB |      | ok     |      | Bton | MA    |
  | .ZZC | Coco Co  | +20003 | c@    | ccC |      | ok,co  |      | Cton | CA    |
  | .ZZF | For Co   | +20006 | f@    | ccF |      | co     |      | Fton | FL    |
  And these "r_boxes":
  | uid  | code |*
  | .ZZC | devC |
  And these "u_relations":
  | main | agent | num | permission |*
  | .ZZC | .ZZA  |   1 | buy        |
  | .ZZC | .ZZB  |   2 | scan       |
  | .ZZF | .ZZA  |   1 | manage     |

# GET /idPhoto

Scenario: The app asks to show a customer ID photo
  Given next random code is "A123456789B123456789C123456789D123456789E123456789etc"
  And var "code" is "A123456789B123456789C123456789D123456789E123456789K6VMDJJccB" encrypted
  And member ".ZZB" has photo "pictureB"
  When app gets "idPhoto" with:
  | accountId  | code  |*
  | K6VMDJI    | %code |
  Then we reply "got" with: "pictureB"

Scenario: The app asks to show a customer ID photo with no accountId
  And var "code" is "A123456789B123456789C123456789D123456789E123456789K6VMDJJccB" encrypted
  When app gets "idPhoto" with:
  | accountId  | code  |*
  |            | %code |
  Then we reply "syntax" with: "?"

Scenario: The app asks to show a customer ID photo with no code
  When app gets "idPhoto" with:
  | accountId  | code  |*
  | K6VMDJI    |       |
  Then we reply "syntax" with: "?"

Scenario: The app asks to show a customer ID photo with bad accountId
  And var "code" is "A123456789B123456789C123456789D123456789E123456789K6VMDJJccB" encrypted
  When app gets "idPhoto" with:
  | accountId   | code  |*
  | whatever    | %code |
  Then we reply "notfound" with: "?"

Scenario: The app asks to show a customer ID photo with bad code
  When app gets "idPhoto" with:
  | accountId  | code     |*
  | K6VMDJI    | whatever |
  Then we reply "notfound" with: "?"

Scenario: The app asks to show a customer ID photo with bad otherId
  And var "code" is "A123456789B123456789C123456789D123456789E123456789.whatever" encrypted
  When app gets "idPhoto" with:
  | accountId  | code  |*
  | K6VMDJI    | %code |
  Then we reply "notfound" with: "?"
