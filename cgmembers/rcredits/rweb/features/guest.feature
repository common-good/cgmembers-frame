Feature: Guest
AS a member company or administrator
I WANT to track payments from non-members
SO I can accept credit card payments without having to have our own credit card processing deal.

Setup:
  Given members:
  | uid  | fullName | phone        | address | city  | state  | zip | country  | postalAddr | floor | flags              | coFlags   | emailCode |*
  | .ZZA | Abe One  | +13013013001 | 1 A St. | Atown | Alaska | 01000 | US     | 1 A, A, AK |  -250 | ok,confirmed,debt  |           |           |
  | .ZZB | Bea Two  |              | 2 B St. | Btown | Utah   | 02000 | US     | 2 B, B, UT |  -250 | ok,confirmed,debt  |           |           |
  | .ZZC | Our Pub  | +13333333333 | 3 C St. | Ctown | Cher   |       | France | 3 C, C, FR |     0 | ok,confirmed,co    |           | Cc3       |
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

Scenario: A non-member pays a member company by credit card
  Given button code "BUTTONCODE" for:
  | account | item   | secret |*
  | .ZZC    | apples | Cc3    |
  When member "?" visits "ccpay/code=BUTTONCODE"
  Then we show "Pay Our Pub" with:
  | Pay         |
  | Name        |
  | Phone       |
  | Email       |
  | Country     |
  | Postal Code |
  | Pay         |

  Given next captcha is "37"
  And var "CODE" encrypts:
  | type     | item     | pid | period | amount | coId   |*
  | purchase | apples   | 1   | once   | 123.00 | NEWZZC |
  When member "?" completes "ccpay/code=BUTTONCODE" with:
  | amount | fullName | phone        | email | zip   | payHow | comment  | cq | ca |*
  |    123 | Zee Zot  | 262-626-2626 | z@    | 01301 |      1 | awesome! | 37 | 74 |
  Then these "people":
  | pid | fullName | phone        | email | zip   | state |*
  | 1   | Zee Zot  | +12626262626 | z@    | 01301 | MA    |

  And we redirect to "https://www.paypal.com/cgi-bin/webscr"
  And return URL "/ccpay/op=done&code=CODE"
  
  When member "?" visits "ccpay/op=done&code=CODE"
  Then these "txs2":
  | xid | payee | amount | completed | deposit | pid |*
  | 1   | .ZZC  | 123    | %now      |    %now | 1   |
  And these "txs":
  | eid | xid | payer      | payee | amount | purpose  | type       |*
  | 1   | 1   | %UID_OUTER | .ZZC  | 123    | apples   | %E_OUTER   |
  | 3   | 1   | .ZZC       | cgf   | 3.69   | cc fee   | %E_USD_FEE |
  And we email "purchase-thanks-nonmember" to member "z@" with subs:
  | fullName     | Zee Zot         |**
  | date         | %mdY            |
  | coName       | Our Pub         |
  | coPostalAddr | 3 C, C, FR      |
  | coPhone      | +1 333 333 3333 |
  | amount       | $123            |
  | item         | apples          |
  | noFrame      | 1               |
  And we email "purchase-report" to member ".ZZC" with subs:
  | item         | apples               |**
  | amount       | $123                 |
  | date         | %mdY                 |
  | fromName     | Zee Zot              |
  | fromAddress  | Greenfield, MA 01301 |
  | fromPhone    | +1 262 626 2626      |
  | fromEmail    | z@example.com        |
  | note         | awesome!             |
  And we say "status": "purchase thanks|check it out" with subs:
  | coName | Our Pub |**

Skip Don't allow ACH for guests (for regulatory reasons: the bank wants us to KYC)
# This includes donations to member organizations we don't sponsor, because we don't trust the member organization like we do ourselves.
Scenario: A non-member pays a member company by ACH
  Given button code "BUTTONCODE" for:
  | account | item   | secret |*
  | .ZZC    | apples | Cc3    |
  And next captcha is "37"
  When member "?" completes "ccpay/code=BUTTONCODE" with:
  | amount | fullName | phone        | email | zip   | payHow | comment  | cq | ca |*
  |    123 | Zee Zot  | 262-626-2626 | z@    | 01301 |      0 | awesome! | 37 | 74 |
  Then these "people":
  | pid | fullName | phone        | email | zip   | state |*
  | 1   | Zee Zot  | +12626262626 | z@    | 01301 | MA    |
  And these "txs2":
  | xid | payee | amount | completed | deposit | pid |*
  | 1   | .ZZC  | 123    | %now      |       0 | 1   |
  And these "txs":
  | eid | xid | payer      | payee | amount | purpose    | type       |*
  | 1   | 1   | %UID_OUTER | .ZZC  | 123    | apples     | %E_OUTER   |
  And count "tx_entries" is 4
  And we email "purchase-thanks-nonmember" to member "z@" with subs:
  | fullName     | Zee Zot         |**
  | date         | %mdY            |
  | coName       | Our Pub         |
  | coPostalAddr | 3 C, C, FR      |
  | coPhone      | +1 333 333 3333 |
  | gift         | $123            |
  And we email "purchase-report" to member "" with subs:
  | item         | apples               |**
  | amount       | $123                 |
  | date         | %mdY                 |
  | fromName     | Zee Zot              |
  | fromAddress  | Greenfield, MA 01301 |
  | fromPhone    | +1 262 626 2626      |
  | fromEmail    | z@example.com        |
  | note         |                      |
  And we say "status": "purchase thanks|check it out" with subs:
  | coName | Our Pub |**
Resume
