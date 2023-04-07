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
  | .ZPC | devC |
  And these "u_relations":
  | main | agent | num | permission |*
  | .ZPC | .ZPA  |   1 | buy        |
  | .ZPC | .ZPB  |   2 | scan       |
  | .ZPF | .ZPA  |   1 | manage     |

# POST /transactions

Scenario: The app asks to charge a customer
  When app posts "transactions" with:
  | deviceId | amount | actorId | otherId | description | created | proof                       | offline |*
  | devC     | 123    | K6VMDCC | K6VMDCB | stuff       | %now    | K6VMDCC123.00K6VMDCBccB%now | false   |
  Then we reply "ok" with JSON:
  | ok    | message                   |*
  | true  | You charged Bea Two $123. |

Scenario: The app asks to charge a customer with a missing parameter
  When app posts "transactions" with:
  | deviceId | amount | actorId | otherId | description | created | proof                       | offline |*
  | devC     | 123    | K6VMDCC | K6VMDCB |             | %now    | K6VMDCC123.00K6VMDCBccB%now | false   |
  Then we reply "syntax" with: "?"

Scenario: The app asks to charge a customer with a bad actorId
  When app posts "transactions" with:
  | deviceId | amount | actorId | otherId | description | created | proof                       | offline |*
  | devC     | 123    | K6VMDCX | K6VMDCB | stuff       | %now    | K6VMDCC123.00K6VMDCBccB%now | false   |
  Then we reply "unauth"
  
Scenario: The app asks to charge a customer with a bad otherId
  When app posts "transactions" with:
  | deviceId | amount | actorId | otherId    | description | created | proof                       | offline |*
  | devC     | 123    | K6VMDCC | K6VMDCXccx | stuff       | %now    | K6VMDCC123.00K6VMDCBccB%now | false   |
  Then we reply "ok" with JSON:
  | ok    | message                                 |*
  | false | That is not an active %PROJECT account. |

Scenario: The app asks to charge a customer with a bad otherId offline
  When app posts "transactions" with:
  | deviceId | amount | actorId | otherId    | description | created | proof                       | offline |*
  | devC     | 123    | K6VMDCC | K6VMDCXccx | stuff       | %now    | K6VMDCC123.00K6VMDCBccB%now | true    |
  Then we reply "ok"
  And we tell Admin "bad card code" with subs:
  | acctId     | code |*
  | K6VMDCXccx |      |
  And these "tx_bads":
  | deviceId | amount | actorId | otherId    | description | created | proof                       | offline |*
  | devC     | 123    | K6VMDCC | K6VMDCXccx | stuff       | %now    | K6VMDCC123.00K6VMDCBccB%now | 1       |
  
Scenario: The app asks to charge a customer with a bad amount
  When app posts "transactions" with:
  | deviceId | amount | actorId | otherId | description | created | proof                       | offline |*
  | devC     | 1.2.3  | K6VMDCC | K6VMDCB | stuff       | %now    | K6VMDCC123.00K6VMDCBccB%now | false   |
  Then we reply "syntax" with: "?"

Scenario: The app asks to charge a customer with an amount out of range
  When app posts "transactions" with:
  | deviceId | amount             | actorId | otherId | description | created | proof                       | offline |*
  | devC     | %(%MAX_AMOUNT + 1) | K6VMDCC | K6VMDCB | stuff       | %now    | K6VMDCC123.00K6VMDCBccB%now | false   |
  Then we reply "syntax" with: "?"

Scenario: The app asks to charge a customer with a bad date
  When app posts "transactions" with:
  | deviceId | amount | actorId | otherId | description | created | proof                       | offline |*
  | devC     | 123    | K6VMDCC | K6VMDCB | stuff       | abc     | K6VMDCC123.00K6VMDCBccB%now | false   |
  Then we reply "syntax" with: "?"

Scenario: The app asks to charge a customer with a date out of range
  When app posts "transactions" with:
  | deviceId | amount | actorId | otherId | description | created | proof                       | offline |*
  | devC     | 123    | K6VMDCC | K6VMDCB | stuff       | 123     | K6VMDCC123.00K6VMDCBccB%now | false   |
  Then we reply "syntax" with: "?"

Scenario: The app asks to charge a customer with a bad proof
  When app posts "transactions" with:
  | deviceId | amount | actorId | otherId | description | created | proof | offline |*
  | devC     | 123    | K6VMDCC | K6VMDCB | stuff       | %now    | ccX   | false   |
  Then we reply "syntax" with: "?"

Scenario: The app asks to charge a customer who has insufficient funds
  When app posts "transactions" with:
  | deviceId | amount | actorId | otherId | description | created | proof                         | offline |*
  | devC     | 12300  | K6VMDCC | K6VMDCB | stuff       | %now    | K6VMDCC12300.00K6VMDCBccB%now | false   |
  Then we reply "ok" with JSON:
  | ok    | message                                                                                    |*
  | false | Bea Two is $11,300 short for this request (do NOT try again). Available balance is $1,000. |
