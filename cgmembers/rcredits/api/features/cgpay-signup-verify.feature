Feature: CGPay signup-verify
AS the node cgpay app
I WANT PHP to complete signup when I pass a verified email code and a strong password
SO a new member can set their password and be signed in on this device.

Setup:
  Given members:
  | uid  | fullName | email | phone        | flags        |*
  | .ZPD | Dee Four | d@    | +14135550104 | confirmed,ok |

# Rule: a weak password is refused and the code stays usable, so the member can try another

Scenario: The node app completes signup with a weak password
  Given member ".ZPD" one-time password is "SignupCode-ZPD-32"

  When cgpay posts "signup-verify" with:
  | uid  | code              | newPassword |*
  | .ZPD | SignupCode-ZPD-32 | newpass99   |
  Then we reply "syntax" with field error "newPassword": "weak pass"
  And member ".ZPD" one-time password is "SignupCode-ZPD-32"
