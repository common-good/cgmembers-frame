Feature: PartnerSignup
AS a newbie
I WANT to open a Common Good account
SO I can pay a partner company
# Note that "member" in the scenarios below means new member (newbie).

Setup:
  Given members:
  | uid  | fullName | flags  | emailCode | website |*
  | .ZZC | Our Pub  | ok,co  | Ccode     | z.ot    |
  | .ZZZ | Zeta Zot | ok     | Zcode     |         |   
  And member is logged out

Scenario: A newbie visits the registration page sent by a partner
  Given next random code is "WHATEVER"
  When someone posts to page "signup" with:
  | partner | .ZZC |
  | partnerCode | Ccode |
  | customer | Abc-12345 |
  | fullName | Abe One |
  | email | a@ |
  | phone | 413-253-0000 |
  | address | 1 A St. |
  | city | Agawam |
  | state | MA |
  | zip | 01001 |
  | address2 | POB 1 |
  | city2 | Agawam |
  | state2 | MA |
  | zip2 | 01001 |
  | source | radio |
  | qid |  |
  Then members:
  | uid  | fullName | email | phone        | zip   | flags | address | city   | state | postalAddr              |*
  | .AAA | Abe One  | a@    | +14132530000 | 01001 |       | 1 A St. | Agawam | MA    | POB 1, Agawam, MA 01001 |
  And relations:
  | reid | main | agent | flags    |*
  | .AAA | .ZZC | .AAA  | customer |
  And we say "status": "partner welcome" with subs:
  | partner |*
  | Our Pub |
  And member ".AAA" one-time password is set to "WHATEVER"
  And we show "%PROJECT Agreement"
  And member ".AAA" steps left "agree preferences fund verifyemail"
  
  When member "?" visits page "signup/reid=.AAA&customer=Abc-12345"
  Then we say "error": "old partner link"

Scenario: A member visits the registration page sent by a partner
  Given members:
  | uid  | fullName | legalName | email | phone        | zip   | country | state | city   | flags     | floor | address | postalAddr               | tenure | helper |*
  | .AAA | Abe One  | Abe One   | a@    | +14132530000 | 01001 | US      | MA    | Agawam | member    | 0     |    1 A St. | POB 1, Agawam, MA 01001 |     18 | .ZZC   |
  When someone posts to page "signup" with:
  | partner | .ZZC |
  | partnerCode | Ccode |
  | customer | Abc-12345 |
  | fullName | Abe One |
  | email | a@ |
  | phone | 413-253-0000 |
  | address | 1 A St. |
  | city | Agawam |
  | state | MA |
  | zip | 01001 |
  | address2 | POB 1 |
  | city2 | Agawam |
  | state2 | MA |
  | zip2 | 01001 |
  | source | radio |
  | qid | NEWAAA |
  Then relations:
  | reid | main | agent | flags    |*
  | .AAA | .ZZC | .AAA  | customer |
  And we say "status": "new customer done" with subs:
  | partnerName |*
  | Our Pub     |