Feature: Transactions
AS a member
I WANT to charge (or pay) a person who presents their CG card for payment
SO we will be square

Setup:
  Given members:
  | uid  | fullName | phone  | email | cc  | cc2  | flags  | pass | city | state | balance |*
  | .ZZA | Abe One  | +20001 | a@    | ccA | ccA2 | ok     | Aa1  | Aton | AL    | 1000    |
  | .ZZB | Bea Two  | +20002 | b@    | ccB |      | ok     |      | Bton | MA    | 1000    |
  | .ZZC | Coco Co  | +20003 | c@    | ccC |      | ok,co  |      | Cton | CA    | 1000    |
  | .ZZF | For Co   | +20006 | f@    | ccF |      | co     |      | Fton | FL    | 1000    |
  And these "r_boxes":
  | uid  | code |*
  | .ZZC | devC |
  And these "u_relations":
  | main | agent | num | permission |*
  | .ZZC | .ZZA  |   1 | buy        |
  | .ZZC | .ZZB  |   2 | scan       |
  | .ZZF | .ZZA  |   1 | manage     |

# POST /comments

Scenario: The app asks to submit a comment
  When app posts "comments" with:
  | deviceId | actorId | otherId    | text  | created |*
  | devC     | K6VMDJK | K6VMDJJccB | stuff | %now    |
  Then we tell Admin "Comment from Coco Co" with subs:
  | qid    | text  | created |*
  | NEWZZC | stuff | %mdY    |
  And we reply "ok"

Scenario: The app asks to submit a comment with a missing parameter
  When app posts "comments" with:
  | deviceId | actorId | otherId    | text  | created |*
  | devC     | K6VMDJK | K6VMDJJccB |       | %now    |
  Then we reply "syntax" with: "?"

Scenario: The app asks to submit a comment with a bad actorId
  When app posts "comments" with:
  | deviceId | actorId | otherId    | text  | created |*
  | devC     | K6VMDJX | K6VMDJJccB | stuff | %now    |
  Then we reply "unauth"
