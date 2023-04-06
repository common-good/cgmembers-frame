Feature: ID Photo
AS a member
I WANT to securely identify a customer who presents their CG card for payment
SO I can subsequently charge them

Setup:
  Given members:
  | uid  | fullName | phone  | email | cc  | cc2  | flags  | pass | city | state |*
  | .ZPA | Abe One  | +20001 | a@    | ccA | ccA2 | ok     | Aa1  | Aton | AL    |
  | .ZPB | Bea Two  | +20002 | b@    | ccB |      | ok     |      | Bton | MA    |
  | .ZPC | Coco Co  | +20003 | c@    | ccC |      | ok,co  |      | Cton | CA    |
  | .ZPF | For Co   | +20006 | f@    | ccF |      | co     |      | Fton | FL    |
  And these "r_boxes":
  | uid  | code |*
  | .ZPC | devC |
  And these "u_relations":
  | main | agent | num | permission |*
  | .ZPC | .ZPA  |   1 | buy        |
  | .ZPC | .ZPB  |   2 | scan       |
  | .ZPF | .ZPA  |   1 | manage     |

# GET /idPhoto

Scenario: The app asks to show a customer ID photo
  Given member ".ZPB" has photo "pictureB"
  When app gets "idPhoto" with:
  | deviceId | actorId | otherId    |*
  | devC     | K6VMDCC | K6VMDCBccB |
  Then we reply "got" with: "pictureB"

Scenario: The app asks to show a customer ID photo with no otherId
  When app gets "idPhoto" with:
  | deviceId | actorId | otherId    |*
  | devC     | K6VMDCC |            |
  Then we reply "syntax" with: "?"

Scenario: The app asks to show a customer ID photo with no actorId
  When app gets "idPhoto" with:
  | deviceId | actorId | otherId    |*
  | devC     |         | K6VMDCBccB |
  Then we reply "syntax" with: "?"

Scenario: The app asks to show a customer ID photo with bad otherId
  When app gets "idPhoto" with:
  | deviceId | actorId | otherId    |*
  | devC     | K6VMDCC | whatever   |
  Then we reply "notfound" with: "?"

Scenario: The app asks to show a customer ID photo with bad actorId
  When app gets "idPhoto" with:
  | deviceId | actorId  | otherId    |*
  | devC     | whatever | K6VMDCBccB |
  Then we reply "unauth" with: "?"

Scenario: The app asks to show a customer ID photo with bad deviceId
  When app gets "idPhoto" with:
  | deviceId | actorId | otherId    |*
  | whatever | K6VMDCC | K6VMDCBccB |
  Then we reply "unauth" with: "?"
