Feature: Transactions
AS a member
I WANT to charge (or pay) a person who presents their CG card for payment
SO we will be square

Setup:
  Given members:
  | uid  | fullName | phone  | email | cc  | cc2  | flags  | pass | city | state | balance |*
  | .ZPA | Abe One  | +20001 | a@    | ccA | ccA2 | ok     | Aa1  | Aton | AL    | 1000    |
  | .ZPB | Bea Two  | +20002 | b@    | ccB |      | ok     |      | Bton | MA    | 1000    |
  | .ZPC | Coco Co  | +20003 | c@    | ccC |      | ok,co  |      | Cton | CA    | 1000    |
  | .ZPF | For Co   | +20006 | f@    | ccF |      | co     |      | Fton | FL    | 1000    |
  And these "r_boxes":
  | uid  | code |*
  | .ZPC | devC |
  And these "u_relations":
  | main | agent | num | permission |*
  | .ZPC | .ZPA  |   1 | buy        |
  | .ZPC | .ZPB  |   2 | scan       |
  | .ZPF | .ZPA  |   1 | manage     |

# POST /comments

Scenario: The app asks to submit a comment
  When app posts "comments" with:
  | deviceId | actorId | otherId    | text  | created |*
  | devC     | K6VMDCC | K6VMDCBccB | stuff | %now    |
  Then we tell Promo "CGPay comment from Coco Co (NEWZPC %mdY)" with subs:
  | qid    | text  | created |*
  | NEWZPC | stuff | %mdY    |
  And we reply "ok"

Scenario: The app asks to submit a comment with a missing parameter
  When app posts "comments" with:
  | deviceId | actorId | otherId    | text  | created |*
  | devC     | K6VMDCC | K6VMDCBccB |       | %now    |
  Then we reply "syntax" with: "?"

Scenario: The app asks to submit a comment with a bad actorId
  When app posts "comments" with:
  | deviceId | actorId | otherId    | text  | created |*
  | devC     | K6VMDCX | K6VMDCBccB | stuff | %now    |
  Then we reply "unauth"
