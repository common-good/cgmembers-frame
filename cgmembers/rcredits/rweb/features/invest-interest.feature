Feature: Invest
AS a member
I WANT to be in the loop for joining an Investment Club
SO I can support local initiatives and get a return on my savings.
 
Setup:
  Given members:
  | uid  | fullName        | floor | flags                      | city | zip   | dob     | federalId |*
  | .ZZA | Abe One         |  -250 | ok,confirmed,debt          | Aton | 01000 | %now    | 123456789 |
  | .ZZB | Bea Two         |  -250 | ok,confirmed,debt,reinvest | Bton | 01000 | %now    | 123456789 |

Scenario: A member expresses interest
  When member ".ZZA" visits page "invest"
  Then we show "Investment Clubs" with:
  | Comments |
  | Keep me in the loop |
  And without:
  | Phone |

Scenario: A non-member expresses interest
  When member "?" visits page "invest/interest"
  Then we show "Investment Clubs" with:
  | Name |
  | Phone |
  | Email |
  | Postal Code |
  | Referred By |
  | Comments |
  | Keep me in the loop |
  
  When member "?" confirms form "invest/interest" with values:
  | fullName | phone        | email | zip   | source | amount | comments |*
  | Dee Four | 413-628-0004 | d@    | 01004 | news   | 123.45 | good!    |
  Then members:
  | uid    | fullName | name    | flags           | phone       | email | zip   | source | iintent | notes               |*
  | NEWAAA | Dee Four | deefour | nonudge, iclubq | 14136280004 | d@    | 01004 | news   | 123.45  | %dmy  self: good!\n |
  