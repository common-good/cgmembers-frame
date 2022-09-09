Feature: Identity
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

# GET /identity

Scenario: The app asks to identify a customer
  When app gets "identity" with:
  | otherId | K6VMDJJccB |**
  Then we reply "got" with JSON:
  | name    | agent | location |*
  | Bea Two |       | Bton, MA |

Scenario: The app asks to identify a customer with no id
  When app gets "identity" with:
  | otherId | |**
  Then we reply "syntax" with: "?"
  
Scenario: The app asks to identify a customer with a bad security code
  When app gets "identity" with:
  | otherId | K6VMDJJccX |**
  Then we reply "notfound" with: "?"

Scenario: The app asks to identify a customer with an id for a nonexistent account
  When app gets "identity" with:
  | otherId | K6VMDJXccB |**
  Then we reply "notfound" with: "?"

Scenario: The app asks to identify a customer with a really bad id
  When app gets "identity" with:
  | otherId | nonsense |**
  Then we reply "notfound" with: "?"
