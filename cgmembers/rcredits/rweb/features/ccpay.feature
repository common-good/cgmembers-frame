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

Scenario: A non-member clicks a link to donate to a ccOk organization by Stripe
  Given button code "buttonCode" for:
  | account | secret | for    |*
  | .ZZC    | Cc3    | donate |
  When member "?" visits "pay/code=%buttonCode"
  Then we show "Donate to Our Pub" with:
  | Donation:    |
  | When:        |
  | Honoring:    |
  | member?      |
  | Name:        |
  | Phone:       |
  | Email:       |
  | Country:     |
  | Postal Code: |
  | cover        |
  | Next         |
  | Pay by:      |
  | Note:        |
  | Donate       |

Scenario: A non-member submits donation information to Stripe
  Given next codes are "strCid123 strId456 secret78"
  When member "?" ajax "stripeSetup" with:
  | amount   | 123          |**
  | period   | once         |
  | for      | donate       |
  | honor    | memory       |
  | honored  | God          |
  | fullName | Zee Zot      |
  | phone    | 262-626-2626 |
  | email    | z@           |
  | zip      | 01301        |
  | country  | US           |
  | notes    | wow!         |
  | coId     | .ZZC         |
  Then these "people":
  | pid | fullName | phone        | email | zip   | state | country | stripeCid | notes                   | source                 |*
  | 1   | Zee Zot  | +12626262626 | z@    | 01301 | MA    | US      | strCid123 | in memory of God - wow! | paid Our Pub (by card) |
  And ajax returns:
  | pid  | purpose  | stripeId | secret   | ok |*
  | 1    | donation | strId456 | secret78 | 1  |

Scenario: A non-member confirms donation intent
  Given these "people":
  | pid | fullName | phone        | email | zip   | state | country | stripeCid | notes                   | source                 |*
  | 1   | Zee Zot  | +12626262626 | z@    | 01301 | MA    | US      | strCid123 | in memory of God - wow! | paid Our Pub (by card) |
  When member "?" ajax "stripeTx" with:
  | amount   | 123          |**
  | period   | once         |
  | for      | donate       |
  | honor    | memory       |
  | honored  | God          |
  | fullName | Zee Zot      |
  | phone    | 262-626-2626 |
  | email    | z@           |
  | zip      | 01301        |
  | country  | US           |
  | notes    | wow!         |
  | coId     | .ZZC         |
  | pid      | 1            |
  | purpose  | donation     |
  | stripeId | strId456     |
  | secret   | secret78     |
  Then these "tx_timed":
  | id | action   | from         | to   | amount | portion | purpose  | payerType   | payer | period | stripeId |*
  | 1  | %ACT_PAY | %MATCH_PAYER | .ZZC | 123    | 0       | donation | %REF_PERSON | 1     | once   | strId456 | 
#  And these "queue":
#  And ajax returns: (encrypted ryP)

  And these "txs2":
  | xid | payee | amount | completed | deposit | pid |*
  | 1   | .ZZC  | 123    | %now      |    %now | 1   |
  And these "txs":
  | eid | xid | payer      | payee | amount | purpose  | cat1       | cat2        | type       |*
  | 1   | 1   | %UID_OUTER | .ZZC  | 123    | donation | AAAAJV     |             | %E_OUTER   |
  | 3   | 1   | .ZZC       | cgf   | 3.69   | cc fee   |            | TX-FEE-BACK | %E_XFEE    |
  And count "txs" is 2
  And we email "gift-thanks-nonmember" to member "z@" with subs:
  | fullName     | Zee Zot         |**
  | date         | %mdY            |
  | coName       | Our Pub         |
  | coPostalAddr | 3 C, C, FR      |
  | coPhone      | +1 333 333 3333 |
  | amount       | $123            |
  | noFrame      | 1               |
  And we email "gift-report" to member ".ZZC" with subs:
  | item         | donation ("in memory of God - wow!") |**
  | amount       | $123                  |
  | date         | %mdY                  |
  | fromName     | Zee Zot               |
  | fromAddress  | , MA 01301            |
  | fromPhone    | +1 262 626 2626       |
  | fromEmail    | z@example.com         |
  And we say "status": "gift thanks|check it out" with subs:
  | coName | Our Pub |**

Scenario: A non-member confirms a donation to a sponsored organization by credit card
  Given these "people":
  | pid | fullName | phone        | email | zip   | state | country | stripeCid | notes                   | source                 |*
  | 1   | Zee Zot  | +12626262626 | z@    | 01301 | MA    | US      | strCid123 | in memory of God - wow! | paid Our Pub (by card) |
  And members have:
  | uid   | coFlags   |*
  | .ZZC  | sponsored |
  And these "tx_rules":
  | id        | 1            |**
  | payer     |              |
  | payerType | %REF_ANYBODY |
  | payee     | .ZZC         |
  | payeeType | %REF_ACCOUNT |
  | from      | %MATCH_PAYEE |
  | to        | cgf          |
  | action    | %ACT_SURTX   |
  | amount    | 0            |
  | portion   | .05          |
  | purpose   | %FS_NOTE     |
  | minimum   | 0            |
  | useMax    |              |
  | amtMax    |              |
  | template  |              |
  | start     | %now         |
  | end       |              |
  | code      |              |
  
  When member "?" ajax "stripeTx" with:
  | amount   | 123          |**
  | period   | once         |
  | for      | donate       |
  | honor    | memory       |
  | honored  | God          |
  | fullName | Zee Zot      |
  | phone    | 262-626-2626 |
  | email    | z@           |
  | zip      | 01301        |
  | country  | US           |
  | notes    | wow!         |
  | coId     | .ZZC         |
  | pid      | 1            |
  | purpose  | donation     |
  | stripeId | strId456     |
  | secret   | secret78     |
  Then these "tx_timed":
  | id | action   | from         | to   | amount | portion | purpose  | payerType   | payer | period | stripeId |*
  | 1  | %ACT_PAY | %MATCH_PAYER | .ZZC | 123    | 0       | donation | %REF_PERSON | 1     | once   | strId456 | 
#  And these "queue":
#  And ajax returns: (encrypted ryP)

  And these "txs2":
  | xid | payee | amount | completed | deposit | pid |*
  | 1   | .ZZC  | 123    | %now      |    %now | 1   |
  And these "txs":
  | eid | xid | payer      | payee | amount | purpose  | cat1       | cat2        | type     |*
  | 1   | 1   | %UID_OUTER | .ZZC  | 123    | donation | AAAAJV     | D-FBO       | %E_OUTER |
  | 3   | 1   | .ZZC       | cgf   | 6.15   | %FS_NOTE | D-FBO      | FS-FEE      | %E_AUX   |
  | 4   | 1   | .ZZC       | cgf   | 3.69   | cc fee   | FBO-TX-FEE | TX-FEE-BACK | %E_XFEE  |

Scenario: A non-member clicks a link to pay a ccOk organization by Stripe
  Given button code "buttonCode" for:
  | account | secret | item  |*
  | .ZZC    | Cc3    | stuff |
  When member "?" visits "pay/code=%buttonCode"
  Then we show "Pay Our Pub" with:
  | Pay:         |
  | member?      |
  | Name:        |
  | Phone:       |
  | Email:       |
  | Country:     |
  | Postal Code: |
  | Next         |
  | Pay by:      |
  | Note:        |
  | Pay          |
And without:
  | When:        |
  | Honoring:    |

Scenario: A non-member submits payment information to Stripe
  Given next codes are "strCid123 strId456 secret78"
  When member "?" ajax "stripeSetup" with:
  | amount   | 123          |**
  | for      |              |
  | item     | stuff        |
  | period   | once         |
  | fullName | Zee Zot      |
  | phone    | 262-626-2626 |
  | email    | z@           |
  | zip      | 01301        |
  | country  | US           |
  | notes    | wow!         |
  | coId     | .ZZC         |
  Then these "people":
  | pid | fullName | phone        | email | zip   | state | country | stripeCid | notes | source                 |*
  | 1   | Zee Zot  | +12626262626 | z@    | 01301 | MA    | US      | strCid123 | wow!  | paid Our Pub (by card) |
  And ajax returns:
  | pid  | purpose | stripeId | secret   | ok |*
  | 1    | stuff   | strId456 | secret78 | 1  |

Scenario: A non-member confirms a payment to a ccOk organization by credit card
  Given these "people":
  | pid | fullName | phone        | email | zip   | state | country | stripeCid | notes | source                 |*
  | 1   | Zee Zot  | +12626262626 | z@    | 01301 | MA    | US      | strCid123 | wow!  | paid Our Pub (by card) |
  And members have:
  | uid   | coFlags   |*
  | .ZZC  | sponsored |
  When member "?" ajax "stripeTx" with:
  | amount   | 123          |**
  | item     | stuff        |
  | period   | once         |
  | fullName | Zee Zot      |
  | phone    | 262-626-2626 |
  | email    | z@           |
  | zip      | 01301        |
  | country  | US           |
  | notes    | wow!         |
  | coId     | .ZZC         |
  | pid      | 1            |
  | purpose  | stuff        |
  | stripeId | strId456     |
  | secret   | secret78     |
  Then these "tx_timed":
  | id | action   | from         | to   | amount | portion | purpose | payerType   | payer | period | stripeId |*
  | 1  | %ACT_PAY | %MATCH_PAYER | .ZZC | 123    | 0       | stuff   | %REF_PERSON | 1     | once   | strId456 | 
#  And these "queue":
#  And ajax returns: (encrypted ryP)

  And these "txs2":
  | xid | payee | amount | completed | deposit | pid |*
  | 1   | .ZZC  | 123    | %now      |    %now | 1   |
  And these "txs":
  | eid | xid | payer      | payee | amount | purpose | cat1       | cat2        | type     |*
  | 1   | 1   | %UID_OUTER | .ZZC  | 123    | stuff   | AAAAJV     | D-FBO       | %E_OUTER |
  | 3   | 1   | .ZZC       | cgf   | 3.69   | cc fee  | FBO-TX-FEE | TX-FEE-BACK | %E_XFEE  |

Scenario: A member donates to a ccOk organization
  Given button code "buttonCode" for:
  | account | secret | for    |*
  | .ZZC    | Cc3    | donate |
  When member ".ZZA" completes "pay/code=%buttonCode" with:
  | amount | note     | period | honor  | honored |*
  |    123 | awesome! | month  | memory | Mike    |
  Then these "txs":
  | eid | xid | payer | payee | amount | purpose  | type       |*
  | 1   | 1   | .ZZA  | .ZZC  | 123    | donation | %E_PRIME   |
  And these "tx_timed":
  | id | action   | from | to   | amount | portion | purpose  | payerType    | payeeType    | period |*
  | 1  | %ACT_PAY | .ZZA | .ZZC | 123    | 0       | donation | %REF_ANYBODY | %REF_ANYBODY | month  |  
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
  | pay | Our Pub | 123    | payment | month  | 1       | 0      |
  Then these "txs":
  | eid | xid | payer | payee | amount | purpose | type       | cat1        | cat2  | flags |*
  | 1   | 1   | .ZZA  | .ZZC  | 123    | payment | %E_PRIME   |             |       | 0     |
  And these "tx_timed":
  | id | action   | from | to   | amount | portion | purpose | payerType    | payeeType    | period | periods |*
  | 1  | %ACT_PAY | .ZZA | .ZZC | 123    | 0       | payment | %REF_ANYBODY | %REF_ANYBODY | month  | 1       |
  And count "tx_entries" is 2
  And we message "you paid" to member ".ZZA" with subs:
  | myName       | Abe One         |**
  | createdDpy   | %mdY            |
  | otherName    | Our Pub         |
  | amount       | $123            |
  | msg          | ?               |
  And we message "paid you linked" to member ".ZZC" with subs:
  | payeePurpose | payment               |**
  | amount       | $123                  |
  | createdDpy   | %mdY                  |
  | otherName    | Abe One               |
  And we say "status": "selfhelp tx|repeats" with subs:
  | myName  | amount | often   |*  
  | Our Pub | $123   | monthly |
