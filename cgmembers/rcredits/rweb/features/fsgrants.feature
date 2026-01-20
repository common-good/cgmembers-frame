Feature: FS Grants
AS a sponsored member company or administrator
I WANT to track expected grants
SO I can be confident that grants are credited promptly and documented properly.

Setup:
  Given members:
  | uid  | fullName | phone        | address | city  | state  | zip | country  | postalAddr | floor | flags              | coFlags  |*
  | .ZZA | Abe One  | +13013013001 | 1 A St. | Atown | Alaska | 01000 | US     | 1 A, A, AK |  -250 | ok,confirmed,admin |          |
  | .ZZB | Bea Two  |              | 2 B St. | Btown | Utah   | 02000 | US     | 2 B, B, UT |  -250 | ok,confirmed,debt  |          |
  | .ZZC | Our Pub  | +13333333333 | 3 C St. | Ctown | Cher   |       | France | 3 C, C, FR |     0 | ok,confirmed,co    | sponsored|
  And these "admins":
  | uid  | vKeyE     | can                                  |*
  | .ZZA | DEV_VKEYE | v,mutualAid,nonmemberTx,region,panel |
  And these "u_relations":
  | main | agent | permission |*
  | .ZZA | .ZZB  | buy        |
  | .ZZB | .ZZA  | read       |
  | .ZZC | .ZZB  | buy        |
  | .ZZC | .ZZA  | manage     |
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
  | purpose   | %FS_NOTE |
  | minimum   | 0            |
  | useMax    |              |
  | amtMax    |              |
  | template  |              |
  | start     | %now         |
  | end       |              |
  | code      |              |
  And these "grants":
  | id | uid  | pid | amount | created  | by    | xid | received | documented |*
  | 1  | .ZZC | 4   | 4000   | %now0-2d | check | 1   | %now-1d  | %now-2d    |
  | 2  | .ZZC | 5   | 5000   | %now0    | ach   |     |          |            |
  | 3  | .ZZC | 6   | 6000   | %now0-3d | wire  |     | %now     |            |
  And these "txs":
  | eid | xid | payer      | payee | amount    | purpose  | cat1        | cat2        | type     |*
  |   1 | 1   | %UID_OUTER | .ZZC  | 100       | grant    |             | D-FBO       | %E_OUTER |
  |   3 | 1   | .ZZC       | cgf   | %WIRE_FEE | wire fee | FBO-TX-FEE  | TX-FEE-BACK | %E_XFEE  |
  |   4 | 1   | .ZZC       | cgf   | 5         | %FS_NOTE | D-FBO       | FS-FEE      | %E_AUX   |
  And these "txs2":
  | xid | payee | amount | completed | deposit | pid | bankAccount |*
  | 1   | .ZZC  | 100    | %now      | %now    | 4   |             |
  And these "people":
  | pid | fullName | email  | phone        | address | city | state | zip   |*
  | 4   | Dee Forn | d@z.co | 444-444-4444 | 4 Da St | Dton | MA    | 01004 |
  | 5   | Eve Fivo | e@z.co | 555-555-5555 | 5 Ed St | Eton | MA    | 01005 |
  | 6   | Flo Sixy | f@z.co | 666-666-6666 | 6 Fr St | Fton | MA    | 01006 |
  Then balances:
  | uid  | balance |*
  | .ZZA |       0 |
  | .ZZB |       0 |
  | .ZZC |      83 |
  | cgf  |      17 |

Scenario: A sponsored partner visits the grants page
  When member ".ZZC" visits "co/grants"
  Then we show "Grants" with:
  | id | uid    | pid      | xid | amount  | by    | documented | received | created  |
  | 2  | ourpub | Eve Fivo |     | 5000.00 | ach   |            |          | %dmqy    |
  | 1  | ourpub | Dee Forn | 1   | 4000.00 | check | %dmqy-2d   | %dmqy-1d | %dmqy-2d |
  | 3  | ourpub | Flo Sixy |     | 6000.00 | wire  |            | %dmqy    | %dmqy-3d |

Scenario: A sponsored partner adds an expected grant
  When member ".ZZC" visits "co/grants/id=add"
  Then we show "Expected Grants" with:
  | Grantor Name: |
  | Email: |
  | Phone: |
  | Postal Addr: |
  | City: |
  | State: |
  | Postal Code: |
  | Expected Amount: |
  | Method: |
  | check |
  | ach |
  | wire |
  | Documented: |
  And without:
  | Received: |
  When member ".ZZC" submits "co/grants/id=add" with:
  | fullName | email | phone        | address | city | state | zip   | amount | by    | documented |*
  | Jay Fund | j@    | 413-999-9999 | 1 J St. | Jton | NJ    | 09999 | 9,000  | check | %mdY       |  
  Then these "people":
  | fullName | email | phone        | address | city | state | zip   | source |*
  | Jay Fund | j@    | 413-999-9999 | 1 J St. | Jton | NJ    | 09999 |        |
  And these "grants":
  | id | uid  | pid | xid | amount | by    | documented | received | created |*
  | 4  | .ZZC | 7   |     | 9000   | check | %now0      |          | %now    |

Scenario: An administrator views an expected grant
  When member "C:A" visits "co/grants/id=3"
  Then we show "Expected Grants" with:
  | Grantor Name:    | Flo Sixy |
  | Email:           | f@z.co |
  | Phone:           | 666 666 6666 |
  | Postal Addr:     | 6 Fr St |
  | City:            | Fton |
  | State:           | Massachusetts |
  | Postal Code:     | 01006 |
  | Expected Amount: | 6000.00 |
  | Received:        | %mdY |
  | Method:          | |
  | check            | |
  | ach              | |
  | wire             | |
  | Documented:      | |

# Rule: When a grant has been received and adequately documented, funds get distributed.

Scenario: A sponsored partner marks a received grant documented
  When member ".ZZC" submits "co/grants/id=3" with:
  | fullName | email  | phone        | address | city | state | zip   | amount | by  | received | documented |*
  | Flo Sixy | f@z.co | 666-666-6666 | 6 Fr St | Fton | MA    | 01006 | 6,000  | ach | %mdY     | %mdY       |
  Then these "txs":
  | eid | xid | payer      | payee | amount | purpose  | cat1        | cat2        | type     |*
  |   5 | 2   | %UID_OUTER | .ZZC  | 6000   | grant    |             | D-FBO       | %E_OUTER |
  |   6 | 2   | .ZZC       | cgf   | 300    | %FS_NOTE | D-FBO       | FS-FEE      | %E_AUX   |
  And these "txs2":
  | xid | payee | amount | completed | deposit | pid | bankAccount |*
  | 2   | .ZZC  | 6000   | %now      | %now    | 6   |             |
  And these "grants":
  | id | uid  | pid | amount | created  | by    | xid | received | documented |*
  | 3  | .ZZC | 6   | 6000   | %now0-3d | ach   | 2   | %now0    | %now0      |
  And we say "status": "funds distributed"
  And we say "status": "info saved"

Scenario: An admin marks a documented grant received
  When member ".ZZC" submits "co/grants/id=2" with:
  | fullName | email  | phone        | address | city | state | zip   | amount | by   | received | documented |*
  | Eve Fivo | e@z.co | 555-555-5555 | 5 Ed St | Eton | MA    | 01005 | 5,000  | wire |          | %mdY-1d    |
  Then these "grants":
  | id | uid  | pid | amount | created  | by    | xid | received | documented |*
  | 2  | .ZZC | 5   | 5000   | %now0    | wire  |     |          | %now0-1d   |
  When member ".ZZA" submits "co/fsgrants/id=2" with:
  | fullName | email  | phone        | address | city | state | zip   | amount | by   | got | received | documented |*
  | Eve Fivo | e@z.co | 555-555-5555 | 5 Ed St | Eton | MA    | 01005 | 5,000  | wire | on  |          | %mdY-1d    |
  Then these "grants":
  | id | uid  | pid | amount | created  | by    | xid | received | documented |*
  | 2  | .ZZC | 5   | 5000   | %now0    | wire  | 2   | %now     | %now0-1d   |
  And these "txs":
  | eid | xid | payer      | payee | amount    | purpose  | cat1        | cat2        | type     |*
  |   5 | 2   | %UID_OUTER | .ZZC  | 5000      | grant    |             | D-FBO       | %E_OUTER |
  |   6 | 2   | .ZZC       | cgf   | %WIRE_FEE | wire fee | FBO-TX-FEE  | TX-FEE-BACK | %E_XFEE  |
  |   7 | 2   | .ZZC       | cgf   | 250       | %FS_NOTE | D-FBO       | FS-FEE      | %E_AUX   |
  And these "txs2":
  | xid | payee | amount | completed | deposit | pid | bankAccount |*
  | 2   | .ZZC  | 5000   | %now      | %now    | 5   |             |
  And we say "status": "funds distributed"
  And we say "status": "info saved"

# Rule: When a grant has been received and is not yet documented, we notify the sponsored partner.

Scenario: An admin marks an udocumented grant received
  When member ".ZZC" submits "co/grants/id=2" with:
  | fullName | email  | phone        | address | city | state | zip   | amount | by   | received | documented |*
  | Eve Fivo | e@z.co | 555-555-5555 | 5 Ed St | Eton | MA    | 01005 | 5,000  | wire |          |            |
  Then these "grants":
  | id | uid  | pid | amount | created  | by    | xid | received | documented |*
  | 2  | .ZZC | 5   | 5000   | %now0    | wire  |     |          |            |
  When member ".ZZA" submits "co/fsgrants/id=2" with:
  | fullName | email  | phone        | address | city | state | zip   | amount | by   | got | received | documented |*
  | Eve Fivo | e@z.co | 555-555-5555 | 5 Ed St | Eton | MA    | 01005 | 5,000  | wire | on  |          |            |
  Then these "grants":
  | id | uid  | pid | amount | created  | by    | xid | received | documented |*
  | 2  | .ZZC | 5   | 5000   | %now0    | wire  |     | %now     |            |
  And we say "status": "sponsee notified"
  And we say "status": "info saved"
  And we message "grantdoc-required" to member ".ZZC" with subs:
  | grantorName | amt    |*
  | Eve Fivo    | $5,000 |

Scenario: An admin marks an udocumented grant received by check
  When member ".ZZC" submits "co/grants/id=2" with:
  | fullName   | Eve Fivo     |**
  | email      | e@z.co       |
  | phone      | 555-555-5555 |
  | address    | 5 Ed St      |
  | city       | Eton         |
  | state      | MA           |
  | zip        | 01005        |
  | amount     | 5,000        |
  | by         | check        |
  | received   |              |
  | documented |              |
  Then these "grants":
  | id | uid  | pid | amount | created  | by    | xid | received | documented | ckNum | ckDate |*
  | 2  | .ZZC | 5   | 5000   | %now0    | check |     |          |            |       |        |

  When member ".ZZA" submits "co/fsgrants/id=2" with:
  | fullName   | Eve Fivo     |**
  | email      | e@z.co       |
  | phone      | 555-555-5555 |
  | address    | 5 Ed St      |
  | city       | Eton         |
  | state      | MA           |
  | zip        | 01005        |
  | amount     | 5,000        |
  | by         | check        |
  | got        | on           |
  | received   |              |
  | documented |              |
  | ckNum      | 123          |
  | ckDate     | %mdY         |
  Then these "grants":
  | id | uid  | pid | amount | created | by    | xid | received | documented | ckNum | ckDate |*
  | 2  | .ZZC | 5   | 5000   | %now0   | check |     | %now     |            | 123   | %now0  |
  And we say "status": "sponsee notified"
  And we say "status": "info saved"
  And we message "grantdoc-required" to member ".ZZC" with subs:
  | grantorName | amt    |*
  | Eve Fivo    | $5,000 |
  
  When member ".ZZC" submits "co/grants/id=2" with:
  | fullName   | Eve Fivo     |**
  | email      | e@z.co       |
  | phone      | 555-555-5555 |
  | address    | 5 Ed St      |
  | city       | Eton         |
  | state      | MA           |
  | zip        | 01005        |
  | amount     | 5,000        |
  | by         | check        |
  | received   | %mdY-1d      |
  | documented | %mdY         |
  | ckNum      | 123          |
  | ckDate     | %mdY-1d      |
  
  Then these "txs":
  | eid | xid | payer      | payee | amount    | purpose                    | cat1        | cat2        | type     |*
  |   5 | 2   | %UID_OUTER | .ZZC  | 5000      | grant (check #123, %mdY-1d) |             | D-FBO       | %E_OUTER |
  |   6 | 2   | .ZZC       | cgf   | 250       | %FS_NOTE                   | D-FBO       | FS-FEE      | %E_AUX   |
  And these "txs2":
  | xid | payee | amount | completed | deposit | pid | bankAccount |*
  | 2   | .ZZC  | 5000   | %now      | %now    | 5   |             |
  And we say "status": "funds distributed"
  And we say "status": "info saved"