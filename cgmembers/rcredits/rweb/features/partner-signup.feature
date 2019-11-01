Feature: PartnerSignup
AS a newbie
I WANT to open a Common Good account
SO I can pay a partner company
# Note that "member" in the scenarios below means new member (newbie).

Setup:
  Given members:
  | uid  | fullName   | flags  | emailCode | website | email | phone        | postalAddr               |*
  | .AIL | Coop Power | ok,co  | Ccode     | z.ot    | c@    | 413-253-0014 | 14 L St., Lton, MA 01014 |
  | .ZZZ | Zeta Zot   | ok     | Zcode     |         | z@    | 413-253-0026 | 26 Z St., Zton, MA 01026 |
  And member is logged out

Scenario: A newbie visits the registration page sent by a partner and opts out of Common Good

  When someone posts to page "partner" with:
  | pid | NEWAIL |
  | type | 0 |
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
  | m_number | 123456 |
  | m_amount | $100-$200 |
  | m_person | Abraham One |
  | cgAccount |  |
  Then members:
  | uid  | fullName | email | phone        | zip   | flags | address | city   | state | postalAddr              |*
  | .AAA | Abe One  | a@    | +14132530000 | 01001 |       | 1 A St. | Agawam | MA    | POB 1, Agawam, MA 01001 |
  And we email "partner-signup" to member "c@" with subs:
  | partnerName | cgAccount | fullName | email | customer | noFrame |*
  | Coop Power  | NEWAAA*   | Abe One  | a@    | E123456  |       1 |
  And we email "partner-contract" to member "a@" with subs:
  | partnerName | partnerAddress           | partnerPhone | partnerEmail  | fullName | code       | noFrame |*
  | Coop Power  | 14 L St., Lton, MA 01014 | 413-253-0014 | c@example.com | Abe One  | TESTDOCODE |       1 |
  And we show "Sign the Contract"
  And member ".AAA" steps left "fund agree preferences partnerend"

  When member ".AAA" visits page "partner/signed=TESTDOCODE"
  Then we show "Source of Funding"
  
  When member ".AAA" completes form "partner/signed=TESTDOCODE" with values:
  | routingNumber | bankAccount | processor |*
  | 211870281     | 1234        |         1 |
  Then we show "Congratulations!"
  And without:
  | "Continue to %PROJECT" |
  And we email "partner-report" to member "c@" with subs:
  | partnerName | fullName | customer | processor            | email         | cgAccount | extra |*
  | Coop Power  | Abe One  | E123456  | Arizona Bank & Trust | a@example.com | (none)    | You will need to delete the %PROJECT account ID in your database. |
  And we email "partner-end" to member "a@" with subs:
  | partnerName | partnerAddress           | partnerPhone | partnerEmail  | noFrame | extra |*
  | Coop Power  | 14 L St., Lton, MA 01014 | 413-253-0014 | c@example.com |       1 |       |

Scenario: A newbie visits the registration page sent by a partner and chooses Common Good
  When someone posts to page "partner" with:
  | pid | NEWAIL |
  | type | 0 |
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
  | m_number | 123456 |
  | m_amount | $100-$200 |
  | m_person | Abraham One |
  | cgAccount |  |
  Then members:
  | uid  | fullName | email | phone        | zip   | flags | address | city   | state | postalAddr              |*
  | .AAA | Abe One  | a@    | +14132530000 | 01001 |       | 1 A St. | Agawam | MA    | POB 1, Agawam, MA 01001 |
  And member ".AAA" steps left "fund agree preferences partnerend"

  When member ".AAA" visits page "partner/signed=TESTDOCODE"
  Then we show "Source of Funding"
  
  When member ".AAA" completes form "partner/signed=TESTDOCODE" with values:
  | routingNumber | bankAccount | processor |*
  | 211870281     | 1234        |         0 |
  Then we show "%PROJECT Agreement"
  And member ".AAA" steps left "agree preferences partnerend"

  When member ".AAA" completes form "community/agreement" with values:
  | op | I Agree |**
  Then we show "Account Preferences"
  And we say "status": "info saved|step completed"
  And member ".AAA" steps left "preferences partnerend"

  When member ".AAA" completes form "settings/preferences" with values:
  | roundup | crumbs | notices | statements | nosearch | secretBal |*
  |       1 |      2 | monthly | electronic |        0 |         1 |
  Then we show "Congratulations!" with:
  | Continue to %PROJECT |
  And we say "status": "info saved|step completed"
  And relations:
  | reid | main | agent | flags    | code    |*
  | .AAA | .AIL | .AAA  | customer | E123456 |
  And member ".AAA" steps left ""
  And we email "partner-report" to member "c@" with subs:
  | partnerName | fullName | customer | processor | email         | cgAccount | extra |*
  | Coop Power  | Abe One  | E123456  | %PROJECT  | a@example.com | NEWAAA    |       |
  And we email "partner-end" to member "a@" with subs:
  | partnerName | partnerAddress           | partnerPhone | partnerEmail  | noFrame | extra |*
  | Coop Power  | 14 L St., Lton, MA 01014 | 413-253-0014 | c@example.com |       1 |     ? |
  
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
  And member ".AAA" has done step "fund" 
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
  | m_number  | 123456 |
  | m_amount  | $100-$200 |
  | m_person  | Abraham One |
  | cgAccount | NEWAAA |
  Then we email "partner-contract" to member "a@" with subs:
  | partnerName | partnerAddress           | partnerPhone | partnerEmail  | fullName | code       | noFrame |*
  | Coop Power  | 14 L St., Lton, MA 01014 | 413-253-0014 | c@example.com | Abe One  | TESTDOCODE |       1 |
  And we show "Sign the Contract"
  And member ".AAA" steps left ""  
  
  When member ".AAA" visits page "partner/signed=TESTDOCODE"
  Then we show "Congratulations!"
  And without:
  | Continue to %PROJECT |
  And relations:
  | reid | main | agent | flags    | code    |*
  | .AAA | .AIL | .AAA  | customer | E123456 |
  And we email "partner-report" to member "c@" with subs:
  | partnerName | fullName | customer | processor | email         | cgAccount | extra |*
  | Coop Power  | Abe One  | E123456  | %PROJECT  | a@example.com | NEWAAA    |       |
  And we email "partner-end" to member "a@" with subs:
  | partnerName | partnerAddress           | partnerPhone | partnerEmail  | noFrame | extra |*
  | Coop Power  | 14 L St., Lton, MA 01014 | 413-253-0014 | c@example.com |       1 |       |

Scenario: A company visits the registration page sent by a partner
  Given next random code is "WHATEVER"
  When someone posts to page "partner" with:
  | pid | NEWAIL |
  | type | 1 |
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
  | m_number | 123456 |
  | m_amount | $100-$200 |
  | m_person | Go Co Ltd |
  | cgAccount |  |
  Then members:
  | uid  | fullName | contact  | email | phone        | zip   | flags      | address | city   | state | postalAddr             |*
  | .AAA | Go Co    | Al Aargh | g@    | +14132530000 | 01001 | co,nonudge,depends | 1 A St. | Agawam | MA    | POB 1, Agawam, MA 01001 |
  And member ".AAA" steps left "signup fund agree partnerend"

  When member ".AAA" visits page "partner/signed=TESTDOCODE"
  Then we show "Source of Funding"

  Given member ".AAA" has done step "agree"  
  When member ".AAA" completes form "partner/signed=TESTDOCODE" with values:
  | routingNumber | bankAccount | processor |*
  | 211870281     | 1234        |         0 |
  Then we show "Open a Trial Company Account"
  And with:
  | Selling:    |
  | Own Phone:  |
  And without:
  | Your Name:  |
  | Company:    |
  | Email:      |
  | Company Phone: |
  | Source      |
  | Account ID: |
  And member ".AAA" steps left "signup partnerend"

  When member ".AAA" completes form "signup-co" with values:
  | contact  | fullName | zip   | phone        | email | selling | source | ownPhone | qid |*
  | Al Aargh | Go Co    | 01001 | 413-253-0000 | g@    | carts   | chas   |        1 |     |
  
  Then we show "Congratulations!" with:
  | Continue to %PROJECT |
  And we say "status": "info saved|step completed"
  And relations:
  | reid | main | agent | flags    | code    |*
  | .AAA | .AIL | .AAA  | customer | E123456 |
  And member ".AAA" steps left ""
  And we email "partner-report" to member "c@" with subs:
  | partnerName | fullName | customer | processor | email         | cgAccount | extra |*
  | Coop Power  | Go Co    | E123456  | %PROJECT  | g@example.com | NEWAAA    |     ? |
  And we email "partner-end" to member "g@" with subs:
  | partnerName | partnerAddress           | partnerPhone | partnerEmail  | noFrame | extra |*
  | Coop Power  | 14 L St., Lton, MA 01014 | 413-253-0014 | c@example.com |       1 |     ? |
