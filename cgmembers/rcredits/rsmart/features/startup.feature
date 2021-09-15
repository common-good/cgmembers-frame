Feature: Start up
AS a member
I WANT to run the Common Good POS app on my device
SO I can use it to charge customers through the Common Good system.

Setup:
  Given members:
  | uid  | fullName | phone  | email | cc  | cc2  | flags         |*
  | .ZZA | Abe One  | +20001 | a@    | ccA | ccA2 | ok            |
  | .ZZB | Bea Two  | +20002 | b@    | ccB |      | ok            |
  | .ZZC | Coco Co  | +20003 | c@    | ccC |      | ok,co         |
  | .ZZF | For Co   | +20006 | f@    | ccF |      | co            |
  And these "r_boxes":
  | uid  | code |*
  | .ZZC | devC |
  And these "u_relations":
  | main | agent | num | permission |*
  | .ZZC | .ZZA  |   1 | buy        |
  | .ZZC | .ZZB  |   2 | scan       |
  | .ZZF | .ZZA  |   1 | scan       |

Scenario: Device requests a bad op
  When agent "C:A" asks device "devC" for op %random with: ""
  Then we return error "bad op"

Scenario: Device should have an identifier
  When agent "C:A" asks device "" for op "charge" with:
  | member | code |*
  | .ZZB   | ccB  |
  Then we return error "missing device"

Skip : It might be a code from a different server
Scenario: Device gives a bad code
  When agent "C:A" asks device %random for op "charge" with:
  | member | code |*
  | .ZZB   | ccB  |
  Then we return error "unknown device"
Resume

Scenario: An Agent for an inactive company tries an op
  When agent "F:A" asks device "devC" for op "charge" with: ""
  Then we return error "company inactive"