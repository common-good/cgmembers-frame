Feature: Transactions
AS a member
I WANT to charge (or pay) a person who presents their CG card for payment
SO we will be square

Setup:
  Given members:
  | uid  | fullName | phone  | email | cc  | cc2  | flags  | pass | city | state | balance |*
  | .ZZA | Abe One  | +20001 | a@    | ccA | ccA2 | ok     | Aa1  | Aton | AL    | 1000    |
  | .ZZB | Bea Two  | +20002 | b@    | ccB |      | ok     |      | Bton | MA    | 1000    |
  | .ZZC | Coco Co  | +20003 | c@    | ccC |      | ok,co  |      | Cton | CA    | 1000    |
  | .ZZF | For Co   | +20006 | f@    | ccF |      | co     |      | Fton | FL    | 1000    |
  And these "r_boxes":
  | uid  | code |*
  | .ZZC | devC |
  And these "u_relations":
  | main | agent | num | permission |*
  | .ZZC | .ZZA  |   1 | buy        |
  | .ZZC | .ZZB  |   2 | scan       |
  | .ZZF | .ZZA  |   1 | manage     |

# POST /transactions

Scenario: The app asks to charge a customer
  When app posts "transactions" with:
  | deviceId | amount | actorId | otherId    | description | created | proof                       | offline |*
  | devC     | 123    | K6VMDJK | K6VMDJJccB | stuff       | %now    | K6VMDJK123.00K6VMDJJccB%now | false   |
  Then we reply "ok" with JSON:
  | ok    | message                   |*
  | true  | You charged Bea Two $123. |

Scenario: The app asks to charge a customer with a missing parameter
  When app posts "transactions" with:
  | deviceId | amount | actorId | otherId    | description | created | proof                       | offline |*
  | devC     | 123    | K6VMDJK | K6VMDJJccB |             | %now    | K6VMDJK123.00K6VMDJJccB%now | false   |
  Then we reply "syntax" with: "?"

Scenario: The app asks to charge a customer with a bad actorId
  When app posts "transactions" with:
  | deviceId | amount | actorId | otherId    | description | created | proof                       | offline |*
  | devC     | 123    | K6VMDJX | K6VMDJJccB | stuff       | %now    | K6VMDJK123.00K6VMDJJccB%now | false   |
  Then we reply "ok" with JSON:
  | ok    | message                                                |*
  | false | This device is not connected to an authorized account. |
  
Scenario: The app asks to charge a customer with a bad otherId
  When app posts "transactions" with:
  | deviceId | amount | actorId | otherId    | description | created | proof                       | offline |*
  | devC     | 123    | K6VMDJK | K6VMDJXccx | stuff       | %now    | K6VMDJK123.00K6VMDJJccB%now | false   |
  Then we reply "ok" with JSON:
  | ok    | message                        |*
  | false | That is not an active account. |
  
Scenario: The app asks to charge a customer with a bad amount
  When app posts "transactions" with:
  | deviceId | amount | actorId | otherId    | description | created | proof                       | offline |*
  | devC     | 1.2.3  | K6VMDJK | K6VMDJJccB | stuff       | %now    | K6VMDJK123.00K6VMDJJccB%now | false   |
  Then we reply "syntax" with: "?"

Scenario: The app asks to charge a customer with an amount out of range
  When app posts "transactions" with:
  | deviceId | amount             | actorId | otherId    | description | created | proof                       | offline |*
  | devC     | %(%MAX_AMOUNT + 1) | K6VMDJK | K6VMDJJccB | stuff       | %now    | K6VMDJK123.00K6VMDJJccB%now | false   |
  Then we reply "syntax" with: "?"

Scenario: The app asks to charge a customer with a bad date
  When app posts "transactions" with:
  | deviceId | amount | actorId | otherId    | description | created | proof                       | offline |*
  | devC     | 123    | K6VMDJK | K6VMDJJccB | stuff       | abc     | K6VMDJK123.00K6VMDJJccB%now | false   |
  Then we reply "syntax" with: "?"

Scenario: The app asks to charge a customer with a date out of range
  When app posts "transactions" with:
  | deviceId | amount | actorId | otherId    | description | created | proof                       | offline |*
  | devC     | 123    | K6VMDJK | K6VMDJJccB | stuff       | 123     | K6VMDJK123.00K6VMDJJccB%now | false   |
  Then we reply "syntax" with: "?"

Scenario: The app asks to charge a customer with a bad proof
  When app posts "transactions" with:
  | deviceId | amount | actorId | otherId    | description | created | proof | offline |*
  | devC     | 123    | K6VMDJK | K6VMDJJccB | stuff       | %now    | ccX   | false   |
  Then we reply "syntax" with: "?"

Scenario: The app asks to charge a customer who has insufficient funds
  When app posts "transactions" with:
  | deviceId | amount | actorId | otherId    | description | created | proof                         | offline |*
  | devC     | 12300  | K6VMDJK | K6VMDJJccB | stuff       | %now    | K6VMDJK12300.00K6VMDJJccB%now | false   |
  Then we reply "ok" with JSON:
  | ok    | message                                                                                    |*
  | false | Bea Two is $11,300 short for this request (do NOT try again). Available balance is $1,000. |
