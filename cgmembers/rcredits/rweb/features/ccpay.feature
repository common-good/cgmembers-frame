Feature: CCPay (see also FBO)
AS a member company authorized to accept CC payments
I WANT to track payments to and from non-members
SO I can accept donations and/or payments.

Setup:
  Given members:
  | uid  | fullName | phone        | address | city  | state  | zip | country  | postalAddr | floor | flags              | coFlags   | emailCode |*
  | .ZZA | Abe One  | +13013013001 | 1 A St. | Atown | Alaska | 01000 | US     | 1 A, A, AK |  -250 | ok,confirmed,debt  |           |           |
  | .ZZB | Bea Two  |              | 2 B St. | Btown | Utah   | 02000 | US     | 2 B, B, UT |  -250 | ok,confirmed,debt  |           |           |
  | .ZZC | Our Pub  | +13333333333 | 3 C St. | Ctown | Cher   |       | France | 3 C, C, FR |     0 | ok,confirmed,co    | ccOk      | Cc3       |
  And these "u_relations":
  | main | agent | permission |*
  | .ZZA | .ZZB  | buy        |
  | .ZZB | .ZZA  | read       |
  | .ZZC | .ZZB  | buy        |
  | .ZZC | .ZZA  | manage     |
  Then balances:
  | uid  | balance |*
  | .ZZA |       0 |
  | .ZZB |       0 |
  | .ZZC |       0 |

Scenario: A non-member donates to a ccOk organization by credit card
  Given button code "buttonCode" for:
  | account | secret |*
  | .ZZC    | Cc3    |
  When member "?" visits "community/donate/code=%buttonCode"
  Then we show "Donate to Our Pub" with:
  | Donation    |
  | Name        |
  | Phone       |
  | Email       |
  | Country     |
  | Postal Code |
  | Donate      |

  Given next captcha is "37"
  And var "code" encrypts:
  | type | item                  | pid | period | amount | coId   |*
  | gift | donation ("awesome!") | 1   | once   | 123.00 | NEWZZC |
  When member "?" completes "community/donate/code=%buttonCode" with:
  | amount | fullName | phone        | email | zip   | payHow | note     | cq | ca |*
  |    123 | Zee Zot  | 262-626-2626 | z@    | 01301 |      1 | awesome! | 37 | 74 |
  Then these "people":
  | pid | fullName | phone        | email | zip   | state |*
  | 1   | Zee Zot  | +12626262626 | z@    | 01301 | MA    |

  And we redirect to "https://www.paypal.com/donate"
  And return URL "/community/donate/op=done&code=%code"
  
  When member "?" visits "community/donate/op=done&code=%code"
  Then these "txs2":
  | xid | payee | amount | completed | deposit | pid |*
  | 1   | .ZZC  | 123    | %now      |    %now | 1   |
  And these "txs":
  | eid | xid | payer      | payee | amount | purpose               | type       |*
  | 1   | 1   | %UID_OUTER | .ZZC  | 123    | donation ("awesome!") | %E_OUTER   |
  | 3   | 1   | .ZZC       | cgf   | 3.69   | cc fee                | %E_XFEE    |
  And we email "gift-thanks-nonmember" to member "z@" with subs:
  | fullName     | Zee Zot         |**
  | date         | %mdY            |
  | coName       | Our Pub         |
  | coPostalAddr | 3 C, C, FR      |
  | coPhone      | +1 333 333 3333 |
  | amount       | $123            |
  | noFrame      | 1               |
  And we email "gift-report" to member ".ZZC" with subs:
  | item         | donation ("awesome!") |**
  | amount       | $123                  |
  | date         | %mdY                  |
  | fromName     | Zee Zot               |
  | fromAddress  | Greenfield, MA 01301  |
  | fromPhone    | +1 262 626 2626       |
  | fromEmail    | z@example.com         |
  And we say "status": "gift thanks|check it out" with subs:
  | coName | Our Pub |**

# ACH is not allowed, but in case it ever is:
Scenario: A non-member donates to a ccOk organization by ACH
  Given button code "buttonCode" for:
  | account | secret |*
  | .ZZC    | Cc3    |
  And next captcha is "37"
  When member "?" completes "community/donate/code=%buttonCode" with:
  | amount | fullName | phone        | email | zip   | payHow | note     | cq | ca |*
  |    123 | Zee Zot  | 262-626-2626 | z@    | 01301 |      0 | awesome! | 37 | 74 |
  Then these "people":
  | pid | fullName | phone        | email | zip   | state |*
  | 1   | Zee Zot  | +12626262626 | z@    | 01301 | MA    |
  And these "txs2":
  | xid | payee | amount | completed | deposit | pid |*
  | 1   | .ZZC  | 123    | %now      |       0 | 1   |
  And these "txs":
  | eid | xid | payer      | payee | amount | purpose               | type       |*
  | 1   | 1   | %UID_OUTER | .ZZC  | 123    | donation ("awesome!") | %E_OUTER   |
  And count "tx_entries" is 2
  And we email "gift-thanks-nonmember" to member "z@" with subs:
  | fullName     | Zee Zot         |**
  | date         | %mdY            |
  | coName       | Our Pub         |
  | coPostalAddr | 3 C, C, FR      |
  | coPhone      | +1 333 333 3333 |
  | amount       | $123            |
  | noFrame      | 1               |
  And we email "gift-report" to member ".ZZC" with subs:
  | item         | donation ("awesome!") |**
  | amount       | $123                  |
  | date         | %mdY                  |
  | fromName     | Zee Zot               |
  | fromAddress  | Greenfield, MA 01301  |
  | fromPhone    | +1 262 626 2626       |
  | fromEmail    | z@example.com         |
  And we say "status": "gift thanks|check it out" with subs:
  | coName | Our Pub |**

Scenario: A member donates to a ccOk organization
  Given button code "buttonCode" for:
  | account | secret |*
  | .ZZC    | Cc3    |
  When member ".ZZA" completes "community/donate/code=%buttonCode" with:
  | amount | note     | period | honor  | honored |*
  |    123 | awesome! | month  | memory | Mike    |
  Then these "txs":
  | eid | xid | payer | payee | amount | purpose               | type       |*
  | 1   | 1   | .ZZA  | .ZZC  | 123    | donation ("awesome!") | %E_PRIME   |
  And these "tx_timed":
  | id | action   | from | to   | amount | portion | purpose                | payerType    | payeeType    | period |*
  | 1  | %ACT_PAY | .ZZA | .ZZC | 123    | 0       | donation ("awesome!")  | %REF_ANYBODY | %REF_ANYBODY | month  |  
  And count "tx_entries" is 2
  And we email "gift-thanks-member" to member "a@" with subs:
  | fullName     | Abe One         |**
  | date         | %mdY            |
  | coName       | Our Pub         |
  | coPostalAddr | 3 C, C, FR      |
  | coPhone      | +1 333 333 3333 |
  | amount       | $123 monthly    |
  | noFrame      |                 |
  And we email "gift-report" to member ".ZZC" with subs:
  | item         | donation ("awesome!") |**
  | amount       | $123 monthly          |
  | date         | %mdY                  |
  | fromName     | Abe One               |
  | fromAddress  | 1 A, A, AK            |
  | fromPhone    | +1 301 301 3001       |
  | fromEmail    | a@example.com         |
  And we say "status": "gift thanks" with subs:
  | coName | Our Pub |**

Scenario: A member pays a ccOk organization
  When member ".ZZA" submits "tx/pay" with:
  | op  | who     | amount | purpose | period | periods | isGift |*
  | pay | Our Pub | 123    | gift    | month  | 1       | 1      |
  Then these "txs":
  | eid | xid | payer | payee | amount | purpose  | type       | cat1        | cat2  |*
  | 1   | 1   | .ZZA  | .ZZC  | 123    | gift     | %E_PRIME   |             |       |
  And these "tx_timed":
  | id | action   | from | to   | amount | portion | purpose | payerType    | payeeType    | period | periods |*
  | 1  | %ACT_PAY | .ZZA | .ZZC | 123    | 0       | gift    | %REF_ANYBODY | %REF_ANYBODY | month  | 1       |
  And count "tx_entries" is 2
  And we email "gift-thanks-member" to member "a@" with subs:
  | fullName     | Abe One         |**
  | date         | %mdY            |
  | coName       | Our Pub         |
  | coPostalAddr | 3 C, C, FR      |
  | coPhone      | +1 333 333 3333 |
  | amount       | $123 monthly    |
  And we email "gift-report" to member ".ZZC" with subs:
  | item         | gift                  |**
  | amount       | $123 monthly          |
  | date         | %mdY                  |
  | fromName     | Abe One               |
  | fromAddress  | 1 A, A, AK            |
  | fromPhone    | +1 301 301 3001       |
  | fromEmail    | a@example.com         |
  And we say "status": "gift thanks" with subs:
  | coName | Our Pub |**
