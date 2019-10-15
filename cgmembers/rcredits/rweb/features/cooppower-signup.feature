Feature: PartnerSignup
AS a newbie
I WANT to open a Common Good account
SO I can pay a partner company
# Note that "member" in the scenarios below means new member (newbie).

Setup:
  Given members:
  | uid  | fullName   | flags  | emailCode | website | email |*
  | .AIL | Coop Power | ok,co  | Ccode     | z.ot    | c@    |
  | .ZZZ | Zeta Zot   | ok     | Zcode     |         | z@    |
  And member is logged out

Scenario: A newbie visits the registration page sent by a partner
  Given next random code is "WHATEVER"
  When someone posts to page "partner" with:
  | pid | NEWAIL |
  | type | 0 |
  | customer | Abc-12345 |
  | fullName | Abe One |
  | orgName |  |
  | email | a@ |
  | phone | 413-253-0000 |
  | address | POB 1 |
  | city | Agawam |
  | state | MA |
  | zip | 01001 |
  | m_address | 1 A St. |
  | m_city | Agawam |
  | m_state | MA |
  | m_zip | 01001 |
  | source | radio |
  | referrer |  |
  | years | 54 |
  | owns  | 1  |
  | m_company | Eversource |
  | m_number | E123456 |
  | m_amount | $100-$200 |
  | m_person | Abraham One |
  | cgAccount |  |
  Then members:
  | uid  | fullName | email | phone        | zip   | flags | address | city   | state | postalAddr              |*
  | .AAA | Abe One  | a@    | +14132530000 | 01001 |       | 1 A St. | Agawam | MA    | POB 1, Agawam, MA 01001 |
  And relations:
  | reid | main | agent | flags             |*
  | .AAA | .AIL | .AAA  | customer, autopay |
  And we say "status": "partner welcome" with subs:
  | partner |*
  | Coop Power |
  And we email "cooppower-signup" to member "c@" with subs:
  | accountId | name    | email | customer | noFrame |*
  | NEWAAA    | Abe One | a@    | E123456  |       1 |
  And member ".AAA" one-time password is set to "WHATEVER"
  And we show "%PROJECT Agreement"
  And member ".AAA" steps left "agree preferences fund verifyemail"
  
  Given step done "agree"
  And step done "preferences"
  And step done "fund"
  
  Given member is logged out
  When member "?" visits page "reset/id=abeone&code=WHATEVER&verify=1"
  Then we show "Verified!"
  And member ".AAA" steps left "partnerend"
  
  When member "?" completes form "reset/id=abeone&code=WHATEVER&verify=1" with values:
  | pass1 | pass2 |*
  |       |       |
  Then we show "%PROJECT Account Completed"
  
Scenario: A member visits the registration page sent by a partner
  Given members:
  | uid        | .AAA |**
  | fullName   | Abe One |
  | legalName  | Abe One |
  | email      | a@ |
  | phone      | +14132530000 |
  | zip        | 01001 |
  | state      | MA |
  | city       | Agawam |
  | flags      | member |
  | floor      | 0 |
  | address    | 1 A St. |
  | postalAddr | POB 1, Agawam, MA 01001 |
  | tenure     | 18 |
  | helper     | .AIL |
  When someone posts to page "partner" with:
  | pid       | NEWAIL |
  | type      | 0 |
  | customer  | Abc-12345 |
  | fullName  | Abe One |
  | orgName   |  |
  | email     | a@ |
  | phone     | 413-253-0000 |
  | address   | POB 1 |
  | city      | Agawam |
  | state     | MA |
  | zip       | 01001 |
  | m_address | 1 A St. |
  | m_city    | Agawam |
  | m_state   | MA |
  | m_zip     | 01001 |
  | source    | radio |
  | referrer  |  |
  | years     | 54 |
  | owns      | 1  |
  | m_company | Eversource |
  | m_number  | E123456 |
  | m_amount  | $100-$200 |
  | m_person  | Abraham One |
  | cgAccount | NEWAAA |
  Then relations:
  | reid | main | agent | flags    |*
  | .AAA | .AIL | .AAA  | customer |
  And we redirect offsite to "docusign"
  
Scenario: A company visits the registration page sent by a partner
  Given next random code is "WHATEVER"
  When someone posts to page "partner" with:
  | pid | NEWAIL |
  | type | 1 |
  | customer | Abc-12345 |
  | fullName | Al Aargh |
  | orgName | Go Co |
  | email | g@ |
  | phone | 413-253-0000 |
  | address | POB 1 |
  | city | Agawam |
  | state | MA |
  | zip | 01001 |
  | m_address | 1 A St. |
  | m_city | Agawam |
  | m_state | MA |
  | m_zip | 01001 |
  | source | radio |
  | referrer |  |
  | years | 54 |
  | owns  | 1  |
  | m_company | Eversource |
  | m_number | E123456 |
  | m_amount | $100-$200 |
  | m_person | Go Co Ltd |
  | cgAccount |  |
  Then members:
  | uid  | fullName | email | phone        | zip   | flags      | address | city   | state | postalAddr              |*
  | .AAA | Go Co    | g@    | +14132530000 | 01001 | co,depends | 1 A St. | Agawam | MA    | POB 1, Agawam, MA 01001 |
  And relations:
  | reid | main | agent | flags             |*
  | .AAA | .AIL | .AAA  | customer, autopay |
  And we say "status": "partner welcome" with subs:
  | partner |*
  | Coop Power |
  And we email "cooppower-signup" to member "c@" with subs:
  | accountId | name    | email | customer | noFrame |*
  | NEWAAA    | Go Co   | g@    | E123456  |       1 |
  And we show "Open a Trial Company Account" with:
  | Your Name     | Al Aargh     |
  | Company       | Go Co        |
  | Postal Code   | 01001        |
  | Company Phone | 413-253-0000 |
  | Email         | g@           |
  And without:
  | Referred By   |  |
  And member ".AAA" steps left "signup discount verifyemail"

  When member ".AAA" completes form "signup-co" with values:
  | source2 | contact  | fullName | zip   | phone        | email | selling | source | ownPhone | qid    |*
  | radio   | Al Aargh | Go Co    | 01004 | 413-253-0004 | g@    | fish    | TV     |        1 |        |  
  Then we show "Get Your Customers Signed Up"
  And we say "status": "info saved|step completed"
  And member ".AAA" steps left "discount verifyemail"
  And member ".AAA" one-time password is set to "WHATEVER"
  
  Given step done "discount"
  
  Given member is logged out
  When member "?" visits page "reset/id=goco&code=WHATEVER&verify=1"
  Then we show "Verified!"
  And member ".AAA" steps left "partnerend"
  
  When member "?" completes form "reset/id=goco&code=WHATEVER&verify=1" with values:
  | pass1 | pass2 |*
  |       |       |
  Then we show "%PROJECT Account Completed"