Feature: Identity
AS a member
I WANT to securely identify a customer who presents their CG card for payment
SO I can subsequently charge them

Setup:
  Given members:
  | uid  | fullName | phone  | email | cc  | cc2  | flags  | pass | city | state | activated |*
  | .ZPA | Abe One  | +20001 | a@    | ccA | ccA2 | ok     | Aa1  | Aton | AL    | 0         |
  | .ZPB | Bea Two  | +20002 | b@    | ccB |      | ok     |      | Bton | MA    | %now0-2w  |
  | .ZPC | Coco Co  | +20003 | c@    | ccC |      | ok,co  |      | Cton | CA    | 0         |
  | .ZPF | For Co   | +20006 | f@    | ccF |      | co     |      | Fton | FL    | 0         |
  And these "u_company":
  | uid  | selling |*
  | .ZPC | stuff   |
  And these "r_boxes":
  | uid  | code |*
  | .ZPC | devC |
  And these "u_relations":
  | main | agent | num | permission |*
  | .ZPC | .ZPA  |   1 | buy        |
  | .ZPC | .ZPB  |   2 | sell       |
  | .ZPF | .ZPA  |   1 | manage     |

# GET /identity

Scenario: The app asks to identify a customer
  When app posts "identity" with:
  | deviceId | actorId | otherId    |*
  | devC     | K6VMDCC | K6VMDCBccB |
  Then we reply "got" with JSON:
  | name    | agent | location | limit | creditLine | avgBalance | trustRatio | since    | selling |*
  | Bea Two |       | Bton, MA |     0 |          0 |          0 |          0 | %now0-2w | [stuff] |
  
Scenario: The app asks to identify a customer without an identifier  
  When app posts "identity" with:
  | deviceId | actorId | otherId |*
  | devC     | K6VMDCC |         |
  Then we reply "syntax" with: "?"
  
Scenario: The app asks to identify a customer with a bad device identifier  
  When app posts "identity" with:
  | deviceId | actorId | otherId    |*
  | whatever | K6VMDCC | K6VMDCBccB |
  Then we reply "unauth" with: "?"

Scenario: The app asks to identify a customer with a bad actor identifier
  When app posts "identity" with:
  | deviceId | actorId  | otherId    |*
  | devC     | whatever | K6VMDCBccB |
  Then we reply "unauth" with: "?"

Scenario: The app asks to identify a customer with a bad security code
  When app posts "identity" with:
  | deviceId | actorId | otherId    |*
  | devC     | K6VMDCC | K6VMDCBccX |
  Then we reply "notfound" with: "?"

Scenario: The app asks to identify a customer with an id for an inactive account
  Given members have:
  | uid  | flags |*
  | .ZPB |       |
  When app posts "identity" with:
  | deviceId | actorId | otherId    |*
  | devC     | K6VMDCC | K6VMDCBccB |
  Then we reply "notfound" with: "?"
  
Scenario: The app asks to identify a customer with an id for a nonexistent account
  When app posts "identity" with:
  | deviceId | actorId | otherId    |*
  | devC     | K6VMDCC | K6VMDCXccB |
  Then we reply "notfound" with: "?"

Scenario: The app asks to identify a customer with a really bad id
  When app posts "identity" with:
  | deviceId | actorId | otherId  |*
  | devC     | K6VMDCC | nonsense |
  Then we reply "notfound" with: "?"
