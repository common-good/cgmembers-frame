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
  Given member ".ZZB" has photo "pictureB"
  When app gets "idPhoto" with:
  | deviceId | actorId | otherId    |*
  | devC     | K6VMDJK | K6VMDJJccB |
  Then we reply "got" with: "pictureB"

Scenario: The app asks to show a customer ID photo with no otherId
  When app gets "idPhoto" with:
  | deviceId | actorId | otherId    |*
  | devC     | K6VMDJK |            |
  Then we reply "syntax" with: "?"

Scenario: The app asks to show a customer ID photo with no actorId
  When app gets "idPhoto" with:
  | deviceId | actorId | otherId    |*
  | devC     |         | K6VMDJJccB |
  Then we reply "syntax" with: "?"

Scenario: The app asks to show a customer ID photo with bad otherId
  When app gets "idPhoto" with:
  | deviceId | actorId | otherId    |*
  | devC     | K6VMDJK | whatever   |
  Then we reply "notfound" with: "?"

Scenario: The app asks to show a customer ID photo with bad actorId
  When app gets "idPhoto" with:
  | deviceId | actorId  | otherId    |*
  | devC     | whatever | K6VMDJJccB |
  Then we reply "unauth" with: "?"

Scenario: The app asks to show a customer ID photo with bad deviceId
  When app gets "idPhoto" with:
  | deviceId | actorId | otherId    |*
  | whatever | K6VMDJK | K6VMDJJccB |
  Then we reply "unauth" with: "?"
