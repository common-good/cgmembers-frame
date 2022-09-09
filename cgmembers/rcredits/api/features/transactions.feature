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
  Given var "code" is "A123456789B123456789C123456789D123456789E123456789K6VMDJKccC" encrypted
  When app posts "transactions" with:
  | deviceId | amount | actorId | otherId | description | created | proof | offline |*
  | %code    | 123    | K6VMDJK | K6VMDJJ | stuff       | %now    | ccB   | false   |
  Then we reply "ok" with: "?"

Scenario: The app asks to charge a customer with a missing parameter
  Given var "code" is "A123456789B123456789C123456789D123456789E123456789K6VMDJKccC" encrypted
  When app posts "transactions" with:
  | deviceId | amount | actorId | otherId | description | created | proof | offline |*
  | %code    | 123    | K6VMDJK | K6VMDJJ |             | %now    | ccB   | false   |
  Then we reply "syntax" with: "?"

Scenario: The app asks to charge a customer with a bad actorId
  Given var "code" is "A123456789B123456789C123456789D123456789E123456789K6VMDJKccC" encrypted
  When app posts "transactions" with:
  | deviceId | amount | actorId | otherId | description | created | proof | offline |*
  | %code    | 123    | K6VMDJX | K6VMDJJ | stuff       | %now    | ccB   | false   |
  Then we reply "notfound" with: "?"
  
Scenario: The app asks to charge a customer with a bad otherId
  Given var "code" is "A123456789B123456789C123456789D123456789E123456789K6VMDJKccC" encrypted
  When app posts "transactions" with:
  | deviceId | amount | actorId | otherId | description | created | proof | offline |*
  | %code    | 123    | K6VMDJK | K6VMDJX | stuff       | %now    | ccB   | false   |
  Then we reply "notfound" with: "?"

Scenario: The app asks to charge a customer with a bad amount
  Given var "code" is "A123456789B123456789C123456789D123456789E123456789K6VMDJKccC" encrypted
  When app posts "transactions" with:
  | deviceId | amount | actorId | otherId | description | created | proof | offline |*
  | %code    | 1.2.3  | K6VMDJK | K6VMDJJ | stuff       | %now    | ccB   | false   |
  Then we reply "syntax" with: "?"

Scenario: The app asks to charge a customer with an amount out of range
  Given var "code" is "A123456789B123456789C123456789D123456789E123456789K6VMDJKccC" encrypted
  When app posts "transactions" with:
  | deviceId | amount             | actorId | otherId | description | created | proof | offline |*
  | %code    | %(%MAX_AMOUNT + 1) | K6VMDJK | K6VMDJJ | stuff       | %now    | ccB   | false   |
  Then we reply "syntax" with: "?"

Scenario: The app asks to charge a customer with a bad date
  Given var "code" is "A123456789B123456789C123456789D123456789E123456789K6VMDJKccC" encrypted
  When app posts "transactions" with:
  | deviceId | amount | actorId | otherId | description | created | proof | offline |*
  | %code    | 123    | K6VMDJK | K6VMDJJ | stuff       | abc     | ccB   | false   |
  Then we reply "syntax" with: "?"

Scenario: The app asks to charge a customer with a date out of range
  Given var "code" is "A123456789B123456789C123456789D123456789E123456789K6VMDJKccC" encrypted
  When app posts "transactions" with:
  | deviceId | amount | actorId | otherId | description | created | proof | offline |*
  | %code    | 123    | K6VMDJK | K6VMDJJ | stuff       | 123     | ccB   | false   |
  Then we reply "syntax" with: "?"

Scenario: The app asks to charge a customer with a bad proof
  Given var "code" is "A123456789B123456789C123456789D123456789E123456789K6VMDJKccC" encrypted
  When app posts "transactions" with:
  | deviceId | amount | actorId | otherId | description | created | proof | offline |*
  | %code    | 123    | K6VMDJK | K6VMDJJ | stuff       | %now    | ccX   | false   |
  Then we reply "notfound" with: "?"

Scenario: The app asks to charge a customer who has insufficient funds
  Given var "code" is "A123456789B123456789C123456789D123456789E123456789K6VMDJKccC" encrypted
  When app posts "transactions" with:
  | deviceId | amount | actorId | otherId | description | created | proof | offline |*
  | %code    | 12300  | K6VMDJK | K6VMDJJ | stuff       | %now    | ccB   | false   |
  Then we reply "denied" with JSON:
  | message | Bea Two is $11,300 short for this request (do NOT try again). Available balance is $1,000. |**
