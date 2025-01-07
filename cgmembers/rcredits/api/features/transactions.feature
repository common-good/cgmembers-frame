Feature: Transactions
AS a member
I WANT to charge (or pay) a person who presents their CG card for payment
SO we will be square

Setup:
  Given members:
  | uid  | fullName | phone  | email | cc  | cc2  | flags  | pass | city | state | balance |*
  | .ZPA | Abe One  | +20001 | a@    | ccA | ccA2 | ok     | Aa1  | Aton | AL    | 1000    |
  | .ZPB | Bea Two  | +20002 | b@    | ccB |      | ok     |      | Bton | MA    | 1000    |
  | .ZPC | Coco Co  | +20003 | c@    | ccC |      | ok,co  |      | Cton | CA    | 1000    |
  | .ZPF | For Co   | +20006 | f@    | ccF |      | co     |      | Fton | FL    | 1000    |
  And these "r_boxes":
  | uid  | code |*
  | .ZPA | devA |
  | .ZPC | devC |
  And these "u_relations":
  | main | agent | num | permission |*
  | .ZPC | .ZPA  |   1 | buy        |
  | .ZPC | .ZPB  |   2 | sell       |
  | .ZPF | .ZPA  |   1 | manage     |

# POST /transactions

Scenario: The app asks to charge a customer
  When app posts "transactions" with:
  | deviceId | amount | actorId  | otherId | description | created | proof                        | offline | pending | version |*
  | devC     | 123    | L6VMDCC0 | K6VMDCB | stuff       | %now    | L6VMDCC0123.00K6VMDCBccB%now | false   |         | 4.0.0   |
  Then we reply "ok" with JSON:
  | ok    | message                   |*
  | true  | You charged Bea Two $123. |
  And these "txs":
  | actorId | uid1 | uid2 | agt1 | agt2 | amt | for2  | created | flags |*
  | .ZPC    | .ZPB | .ZPC | .ZPB | .ZPA | 123 | stuff | %now    |       |

Scenario: The app asks to pay a customer
  When app posts "transactions" with:
  | deviceId | amount | actorId | otherId | description | created | proof                        | pending | offline |*
  | devA     | -123   | K6VMDCA | K6VMDCB | stuff       | %now    | K6VMDCA-123.00K6VMDCBccB%now | false   | false   |
  Then we reply "ok" with JSON:
  | ok    | message                |*
  | true  | You paid Bea Two $123. |
  And these "txs":
  | actorId | uid1 | uid2 | amt | for2  | created | flags |*
  | .ZPA    | .ZPA | .ZPB | 123 | stuff | %now    |       |
  
Scenario: The app asks to undo a charge to a customer
  Given these "txs":
  | xid | actorId | uid1 | uid2 | agt1 | agt2 | amt | for2  | created | flags |*
  | 1   | .ZPC    | .ZPB | .ZPC | .ZPB | .ZPA | 123 | stuff | %now0   |       |
  When app posts "transactions" with:
  | deviceId | amount | actorId  | otherId | description | created | proof                         | pending | offline |*
  | devC     | -123   | L6VMDCC0 | K6VMDCB | stuff       | %now0   | L6VMDCC0123.00K6VMDCBccB%now0 | false   | false   |
  Then we reply "ok" with JSON:
  | ok    | message |*
  | true  | deleted |
  And these "txs":
  | xid | actorId | uid1 | uid2 | agt1 | agt2 | amt  | for2  | created | flags | reversesXid |*
  | 2   | .ZPC    | .ZPB | .ZPC | .ZPB | .ZPA | -123 | stuff | ?       |       | 1           |

Scenario: The app asks to undo a payment to a customer
  Given these "txs":
  | xid | actorId | uid1 | uid2 | amt | for2  | created | flags |*
  | 1   | .ZPA    | .ZPA | .ZPB | 123 | stuff | %now0   |       |
  When app posts "transactions" with:
  | deviceId | amount | actorId | otherId | description | created | proof                         | pending | offline |*
  | devA     | 123    | K6VMDCA | K6VMDCB | stuff       | %now0   | K6VMDCA-123.00K6VMDCBccB%now0 | false   | false   |
  Then we reply "ok" with JSON:
  | ok    | message           |*
  | true  | reversal invoiced |
  And these "tx_requests":
  | payer | payee | amount | purpose | created | reversesXid |*
  | .ZPB  | .ZPA  | 123    | stuff   | ?       | 1           |

Scenario: The app asks to charge a customer and add a tip
  When app posts "transactions" with:
  | deviceId | amount | actorId  | otherId | description | created | proof                        | pending | offline | tip  |*
  | devC     | 123    | L6VMDCC0 | K6VMDCB | stuff       | %now    | L6VMDCC0123.00K6VMDCBccB%now | false   | false   | 12.3 |
  Then we reply "ok" with JSON:
  | ok    | message                   |*
  | true  | You charged Bea Two $123. |
  And these "txs":
  | eid | xid | actorId | uid1 | uid2 | agt1 | agt2 | amt  | for2        | created | flags | type  |*
  | 1   | 1   | .ZPC    | .ZPB | .ZPC | .ZPB | .ZPA | 123  | stuff       | %now    |       | prime |
  | 3   | 1   | .ZPC    | .ZPB | .ZPC | .ZPB | .ZPA | 12.3 | tip (10.0%) | %now    |       | aux   |

Scenario: The app asks to pay a customer and add a tip
  When app posts "transactions" with:
  | deviceId | amount | actorId | otherId | description | created | proof                        | pending | offline | tip  |*
  | devA     | -123   | K6VMDCA | K6VMDCB | stuff       | %now    | K6VMDCA-123.00K6VMDCBccB%now | false   | false   | 2.34 |
  Then we reply "ok" with JSON:
  | ok    | message                |*
  | true  | You paid Bea Two $123. |
  And these "txs":
  | eid | xid | actorId | uid1 | uid2 | amt  | for2       | created | flags | type  |*
  | 1   | 1   | .ZPA    | .ZPA | .ZPB | 123  | stuff      | %now    |       | prime |
  | 3   | 1   | .ZPA    | .ZPA | .ZPB | 2.34 | tip (1.9%) | %now    |       | aux   |

Scenario: The app asks to charge a customer with a missing parameter
  When app posts "transactions" with:
  | deviceId | amount | actorId | otherId | description | created | proof                       | pending | offline |*
  | devC     | 123    | K6VMDCC | K6VMDCB |             | %now    | K6VMDCC123.00K6VMDCBccB%now | false   | false   |
  Then we reply "syntax" with: "?"

Scenario: The app asks to charge a customer with a bad actorId
  When app posts "transactions" with:
  | deviceId | amount | actorId | otherId | description | created | proof                       | pending | offline |*
  | devC     | 123    | K6VMDCX | K6VMDCB | stuff       | %now    | K6VMDCC123.00K6VMDCBccB%now | false   | false   |
  Then we reply "unauth"
  
Scenario: The app asks to charge a customer with a bad otherId
  When app posts "transactions" with:
  | deviceId | amount | actorId | otherId    | description | created | proof                       | pending | offline |*
  | devC     | 123    | K6VMDCC | K6VMDCXccx | stuff       | %now    | K6VMDCC123.00K6VMDCBccB%now | false   | false   |
  Then we reply "ok" with JSON:
  | ok    | message                                 |*
  | false | That is not a %PROJECT account. |

Scenario: The app asks to charge a customer with a bad otherId offline
  When app posts "transactions" with:
  | deviceId | amount | actorId | otherId    | description | created | proof                       | pending | offline |*
  | devC     | 123    | K6VMDCC | K6VMDCXccx | stuff       | %now    | K6VMDCC123.00K6VMDCBccB%now | false   | true    |
  Then we reply "ok"
  And we tell Admin "bad card code" with subs:
  | acctId     | code |*
  | K6VMDCXccx |      |
  And these "tx_bads":
  | deviceId | amount | actorId | otherId    | description | created | proof                       | offline |*
  | devC     | 123    | K6VMDCC | K6VMDCXccx | stuff       | %now    | K6VMDCC123.00K6VMDCBccB%now | 1       |
  
Scenario: The app asks to charge a customer with a bad amount
  When app posts "transactions" with:
  | deviceId | amount | actorId | otherId | description | created | proof                       | pending | offline |*
  | devC     | 1.2.3  | K6VMDCC | K6VMDCB | stuff       | %now    | K6VMDCC123.00K6VMDCBccB%now | false   | false   |
  Then we reply "syntax" with: "?"

Scenario: The app asks to charge a customer with an amount out of range
  When app posts "transactions" with:
  | deviceId | amount             | actorId | otherId | description | created | proof                       | pending | offline |*
  | devC     | %(%MAX_AMOUNT + 1) | K6VMDCC | K6VMDCB | stuff       | %now    | K6VMDCC123.00K6VMDCBccB%now | false   | false   |
  Then we reply "syntax" with: "?"

Scenario: The app asks to charge a customer with a bad date
  When app posts "transactions" with:
  | deviceId | amount | actorId | otherId | description | created | proof                       | pending | offline |*
  | devC     | 123    | K6VMDCC | K6VMDCB | stuff       | abc     | K6VMDCC123.00K6VMDCBccB%now | false   | false   |
  Then we reply "syntax" with: "?"

Scenario: The app asks to charge a customer with a date out of range
  When app posts "transactions" with:
  | deviceId | amount | actorId | otherId | description | created | proof                       | pending | offline |*
  | devC     | 123    | K6VMDCC | K6VMDCB | stuff       | 123     | K6VMDCC123.00K6VMDCBccB%now | false   | false   |
  Then we reply "ok" with JSON:
  | ok    | message                              |*
  | false | transaction date too far in the past |

Scenario: The app asks to charge a customer with a bad proof
  When app posts "transactions" with:
  | deviceId | amount | actorId | otherId | description | created | proof | pending | offline |*
  | devC     | 123    | K6VMDCC | K6VMDCB | stuff       | %now    | ccX   | false   | false   |
  Then we reply "syntax" with: "?"

Scenario: The app asks to charge a customer who has insufficient funds
  When app posts "transactions" with:
  | deviceId | amount | actorId | otherId | description | created | proof                         | pending | offline |*
  | devC     | 12300  | K6VMDCC | K6VMDCB | stuff       | %now    | K6VMDCC12300.00K6VMDCBccB%now | false   | false   |
  Then we reply "ok" with JSON:
  | ok    | message                                                                                    |*
  | false | Bea Two is $11,300 short for this request (do NOT try again). Available balance is $1,000. |
