Feature: Identity
AS a member
I WANT to securely identify a customer who presents their CG card for payment
SO I can subsequently charge them

Setup:
  Given members:
  | uid  | fullName | phone  | email | cc  | cc2  | flags  | pass | city | state | activated |*
  | .ZZA | Abe One  | +20001 | a@    | ccA | ccA2 | ok     | Aa1  | Aton | AL    | 0         |
  | .ZZB | Bea Two  | +20002 | b@    | ccB |      | ok     |      | Bton | MA    | %now0-2w  |
  | .ZZC | Coco Co  | +20003 | c@    | ccC |      | ok,co  |      | Cton | CA    | 0         |
  | .ZZF | For Co   | +20006 | f@    | ccF |      | co     |      | Fton | FL    | 0         |
  And these "u_company":
  | uid  | selling |*
  | .ZZC | stuff   |
  And these "r_boxes":
  | uid  | code |*
  | .ZZC | devC |
  And these "u_relations":
  | main | agent | num | permission |*
  | .ZZC | .ZZA  |   1 | buy        |
  | .ZZC | .ZZB  |   2 | scan       |
  | .ZZF | .ZZA  |   1 | manage     |

# GET /identity

Scenario: The app asks to identify a customer
  When app gets "identity" with:
  | deviceId | actorId | otherId    |*
  | devC     | K6VMDJK | K6VMDJJccB |
  Then we reply "got" with JSON:
  | name    | agent | location | limit | creditLine | avgBalance | trustRatio | since    | items   |*
  | Bea Two |       | Bton, MA |     0 |          0 |          0 |          0 | %now0-2w | [stuff] |
  
Scenario: The app asks to identify a customer without an identifier  
  When app gets "identity" with:
  | deviceId | actorId | otherId |*
  | devC     | K6VMDJK |         |
  Then we reply "syntax" with: "?"
  
Scenario: The app asks to identify a customer with a bad device identifier  
  When app gets "identity" with:
  | deviceId | actorId | otherId    |*
  | xxx      | K6VMDJK | K6VMDJJccB |
  Then we reply "unauth" with: "?"

Scenario: The app asks to identify a customer with a bad actor identifier
  When app gets "identity" with:
  | deviceId | actorId | otherId    |*
  | devC     | xxx     | K6VMDJJccB |
  Then we reply "unauth" with: "?"

Scenario: The app asks to identify a customer with a bad security code
  When app gets "identity" with:
  | deviceId | actorId | otherId    |*
  | devC     | K6VMDJK | K6VMDJJccX |
  Then we reply "notfound" with: "?"

Scenario: The app asks to identify a customer with an id for an inactive account
  Given members have:
  | uid  | flags |*
  | .ZZB |       |
  When app gets "identity" with:
  | deviceId | actorId | otherId    |*
  | devC     | K6VMDJK | K6VMDJJccB |
  Then we reply "notfound" with: "?"
  
Scenario: The app asks to identify a customer with an id for a nonexistent account
  When app gets "identity" with:
  | deviceId | actorId | otherId    |*
  | devC     | K6VMDJK | K6VMDJXccB |
  Then we reply "notfound" with: "?"

Scenario: The app asks to identify a customer with a really bad id
  When app gets "identity" with:
  | deviceId | actorId | otherId  |*
  | devC     | K6VMDJK | nonsense |
  Then we reply "notfound" with: "?"
