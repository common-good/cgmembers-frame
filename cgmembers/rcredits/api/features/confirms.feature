Feature: Confirms
AS a member
I WANT to approve or deny a payment request that arrives in the app
SO I pay only what I mean to pay, once

Setup:
  Given members:
  | uid  | fullName | phone  | email | flags         | risks   | bankAccount  | balance |*
  | .ZPA | Abe One  | +20001 | a@    | ok,balFirst   | hasBank | %T_BANK_ACCT | 1000    |
  | .ZPB | Bea Two  | +20002 | b@    | ok            |         |              | 0       |
  And these "r_boxes":
  | uid  | code |*
  | .ZPA | devA |
  And these "tx_requests":
  | nvid | created | status | amount | payer | payee | for   |*
  |    1 | %today  | open   |      3 | .ZPA  | .ZPB  | stuff |

# POST /confirms

# Rule: A request is paid once, from the member's balance first when they prefer that

Scenario: The app confirms a payment request
  When app posts "confirms" with:
  | deviceId | actorId | id | yesno | version |*
  | devA     | K6VMDCA | 1  | 1     | 4.0.0   |
  Then we reply "ok" with: "?"
  And these "txs":
  | xid | created | amount | payer | payee | purpose |*
  |   1 | %today  |      3 | .ZPA  | .ZPB  | stuff   |
  And balances:
  | uid  | balance |*
  | .ZPA |     997 |
  | .ZPB |       3 |
  And count "txs" is 1

Scenario: The app confirms the same payment request again
  Given app posts "confirms" with:
  | deviceId | actorId | id | yesno | version |*
  | devA     | K6VMDCA | 1  | 1     | 4.0.0   |
  When app posts "confirms" with:
  | deviceId | actorId | id | yesno | version |*
  | devA     | K6VMDCA | 1  | 1     | 4.0.0   |
  Then we reply "ok" with: "?"
  And balances:
  | uid  | balance |*
  | .ZPA |     997 |
  | .ZPB |       3 |

Scenario: The app denies a payment request after paying it
  Given app posts "confirms" with:
  | deviceId | actorId | id | yesno | version |*
  | devA     | K6VMDCA | 1  | 1     | 4.0.0   |
  When app posts "confirms" with:
  | deviceId | actorId | id | yesno | version | whyNot |*
  | devA     | K6VMDCA | 1  | 0     | 4.0.0   | oops   |
  Then we reply "denied" with: "?"

Scenario: The app denies a payment request again
  Given app posts "confirms" with:
  | deviceId | actorId | id | yesno | version | whyNot |*
  | devA     | K6VMDCA | 1  | 0     | 4.0.0   | oops   |
  When app posts "confirms" with:
  | deviceId | actorId | id | yesno | version | whyNot |*
  | devA     | K6VMDCA | 1  | 0     | 4.0.0   | oops   |
  Then we reply "ok" with: "?"
