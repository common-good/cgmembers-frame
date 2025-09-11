Feature: Sponsor
AS a community-spirited not-for-profit organization or project without 501c3 status
I WANT to accept tax-deductible donations
SO I/we can fund our operations with grants and donations from people who need the deduction

Setup:
  Given members:
  | uid  | fullName | flags           |*
  | .ZZA | Abe One  | ok,confirmed    |
  | .ZZB | Bea Two  | ok,confirmed    |
  | .ZZC | Cor Pub  | ok,confirmed,co |
  And members have:
  | uid  | coType  |*
  | .ZZC | %CO_LLC |
  And these "u_relations":
  | main | other | permission |*
  | .ZZC | .ZZA  | read       |
  | .ZZC | .ZZB  | manage     |

Scenario: A non-member applies for fiscal sponsorship
  When member "?" visits "co/sponsor"
  Then we show "Fiscal Sponsorship" with:
  | Your Name     |
  | Your Position |
  | Organization  |
  | Org Phone     |
  | Org Email     |
  | Website       |
  | Country       |
  | Postal Code   |
  | Federal ID    |
  | Account Type  |
  | Referred By   |
  | Mission       |
  | Activities    |
  | Expected Income |
  | Employees     |
  | Contractors   |
  | Checks In     |
  | Checks Out    |
  | Oversight     |
  | Justice       |
  | Comments      |
  | Submit        |
And with:
  | Interested in | |
  | | accepting tax-deductible donations |
  | | payroll service |
  | | management of employee healthcare and other benefits |
  | Do you make grants | |
  | | to individuals? |
  | | to organizations? |
  | | to individuals or organizations in other countries? |
  | If yes to any of the above, please send us your grant-making criteria and process. | |

  When member "?" submits "co/sponsor" with:
  | contact    | Jane Dough |**
  | contactTitle | Director |
  | fullName   | Bread Co   |
  | phone      | 413-987-6543 |
  | email      | jd@example.com |
  | country    | US |
  | zip        | 01301 |
  | federalId  | 12-3456789 |
  | coType     | %CO_CLUB |
  | source     | news |
  | mission    | thrive |
  | activities | do stuff |
  | gross      | 123456.78 |
  | employees  | 9 |
  | checksIn   | 30 |
  | checksOut  | 40 |
  | oversight  | people |
  | justice    | everyone is welcome |
  | comments   | cool! |
  Then we say "status": "got application|meanwhile join"
  And we tell admin "Fiscal Sponsorship Application" with subs:
  | contact    | Jane Dough |**
  | contactTitle | Director |
  | fullName   | Bread Co   |
  | to         | partnerships@%CG_DOMAIN |
  # etc
  | oversight  | people |
  | justice    | everyone is welcome |
  # etc
  And members:
  | uid        | .AAA |**
  | contact    | Jane Dough |
  | contactTitle | Director |
  | fullName   | Bread Co   |
  | legalName  | %CGF_LEGALNAME |
  | federalId  | %CGF_EIN |
  | flags      | co nonudge |
  | coFlags    | sponsored, flip |
  | phone      | 413-987-6543 |
  | email      | jd@example.com |
  | country    | US |
  | zip        | 01301 |
  | coType     | %CO_CLUB |
  | source     | news |
  | mission    | thrive |
  | activities | do stuff |
  | gross      | 123456.78 |
  | employees  | 9 |
  | checksIn   | 30 |
  | checksOut  | 40 |

Scenario: A signed-in individual member applies for fiscal sponsorship
  When member ".ZZA" visits "co/sponsor"
  Then we show "Fiscal Sponsorship" with:
  | Your Name     |
  | Your Position |
  | Organization  |
  | Org Phone     |
  | Org Email     |
  | Country       |
  | Postal Code   |
  | Federal ID    |
  | Account Type  |
  | Referred By   |
  | Mission       |
  | Activities    |
  | Expected Income |
  | Employees     |
  | Checks In     |
  | Checks Out    |
  | Oversight     |
  | Justice       |
  | Comments      |
  | Submit        |

  When member ".ZZA" submits "co/sponsor" with:
  | contact    | Jane Dough |**
  | contactTitle | Director |
  | fullName   | Bread Co   |
  | phone      | 413-987-6543 |
  | email      | jd@example.com |
  | country    | US |
  | zip        | 01301 |
  | federalId  | 12-3456789 |
  | coType     | %CO_CLUB |
  | source     | news |
  | mission    | thrive |
  | activities | do stuff |
  | gross      | 123456.78 |
  | employees  | 9 |
  | checksIn   | 30 |
  | checksOut  | 40 |
  | oversight  | people |
  | justice    | everyone is welcome |
  | comments   | cool! |
  Then we say "status": "got application"
  And we tell admin "Fiscal Sponsorship Application" with subs:
  | contact    | Jane Dough |**
  | contactTitle | Director |
  | fullName   | Bread Co   |
  | to         | partnerships@%CG_DOMAIN |
  # etc
  | oversight  | people |
  | justice    | everyone is welcome |
  # etc
  And members:
  | uid        | .AAA |**
  | contact    | Jane Dough |
  | contactTitle | Director |
  | fullName   | Bread Co   |
  | legalName  | %CGF_LEGALNAME |
  | federalId  | %CGF_EIN |
  | flags      | co nonudge |
  | coFlags    | sponsored, flip |
  | phone      | 413-987-6543 |
  | email      | jd@example.com |
  | country    | US |
  | zip        | 01301 |
  | source     | news |
  | mission    | thrive |
  | activities | do stuff |
  | gross      | 123456.78 |
  | employees  | 9 |
  | checksIn   | 30 |
  | checksOut  | 40 |

Scenario: A signed-in company applies for fiscal sponsorship without manage permission
  When member "C:A" visits "co/sponsor"
  Then we say "error": "no sponsor perm"

Scenario: A signed-in company applies for fiscal sponsorship
  Given members have:
  | uid        | .ZZC |**
  | phone      | 413-987-6543 |
  | email      | c@ |
  | country    | US |
  | zip        | 01301 |
  | source     | news |
  | gross      | 123456.78 |
  | employees  | 9 |
  | coType     | %CO_LLC |
  
  When member "C:B" visits "co/sponsor"
  Then we show "Fiscal Sponsorship" with:
  | Your Position | |
  | Mission       | |
  | Activities    | |
  | Expected Income | 123456.78 |
  | Employees     | 9 |
  | Checks In     | |
  | Checks Out    | |
  | Oversight     | |
  | Justice       | |
  | Comments      | |
  | Submit        | |
  And without:
  | Your Name     |
  | Organization  |
  | Org Phone     |
  | Org Email     |
  | Country       |
  | Postal Code   |
  | Referred By   |
  
  When member "C:B" submits "co/sponsor" with:
  | contactTitle | Director |**
  | mission    | thrive |
  | activities | do stuff |
  | gross      | 123456.78 |
  | employees  | 9 |
  | checksIn   | 30 |
  | checksOut  | 40 |
  | oversight  | people |
  | justice    | everyone is welcome |
  | comments   | cool! |
  Then we say "status": "got application"
  And we tell admin "Fiscal Sponsorship Application" with subs:
  | contact    | Bea Two |**
  | contactTitle | Director |
  | fullName   | Cor Pub |
  | to         | partnerships@%CG_DOMAIN |
  # etc
  | oversight  | people |
  | justice    | everyone is welcome |
  # etc
  And members:
  | uid        | .AAA |**
  | contact    | Bea Two |
  | contactTitle | Director |
  | fullName   | Cor Pub |
  | legalName  | %CGF_LEGALNAME |
  | federalId  | %CGF_EIN |
  | flags      | co nonudge |
  | coFlags    | sponsored, flip |
  | phone      | 413-987-6543 |
  | email      | c@ |
  | country    | US |
  | zip        | 01301 |
  | source     | news |
  | mission    | thrive |
  | activities | do stuff |
  | gross      | 123456.78 |
  | employees  | 9 |
  | checksIn   | 30 |
  | checksOut  | 40 |

Scenario: A fiscally sponsored applicant updates its settings
  Given members:
  | uid        | .ZZF |**
  | contact    | Bea Two |
  | contactTitle | Director |
  | fullName   | Far Co |
  | legalName  | %CGF_LEGALNAME |
  | federalId  | %CGF_EIN |
  | flags      | co,ok |
  | coFlags    | sponsored, flip |
  | phone      | 413-987-6543 |
  | email      | f@ |
  | country    | US |
  | zip        | 01301 |
  | coType     | %CO_LLC |
  | source     | news |
  | mission    | thrive |
  | activities | do stuff |
  | gross      | 123456.78 |
  | employees  | 9 |
  | checksIn   | 30 |
  | checksOut  | 40 |
  And these "u_relations":
  | main | other | permission |*
  | .ZZF | .ZZB  | manage     |
  When member "F:B" visits "co/sponsor"
  Then we show "Fiscal Sponsorship" with:
  | Your Position   | Director  |
  | Mission         | thrive    |
  | Activities      | do stuff  |
  | Expected Income | 123456.78 |
  | Employees       | 9         |
  | Checks In       | 30        |
  | Checks Out      | 40        |
  | Oversight       |           |
  | Justice         |           |
  | Update          |           |
  And without:
  | Your Name     |
  | Organization  |
  | Org Phone     |
  | Org Email     |
  | Country       |
  | Postal Code   |
  | Federal ID    |
  | Account Type  |
  | Referred By   |
  | Comments      |

  When member "F:B" submits "co/sponsor" with:
  | mission    | thrive more |**
  | activities | do more |
  | checksIn   | 35 |
  | checksOut  | 45 |
  Then we say "status": "info saved"
  And members:
  | uid        | .ZZF |**
  | contact    | Bea Two |
  | contactTitle | Director |
  | fullName   | Far Co |
  | legalName  | %CGF_LEGALNAME |
  | federalId  | %CGF_EIN |
  | flags      | co,member,ok,ided |
  | coFlags    | sponsored, flip |
  | phone      | 413-987-6543 |
  | email      | f@ |
  | country    | US |
  | zip        | 01301 |
  | source     | news |
  | mission    | thrive more |
  | activities | do more |
  | gross      | 123456.78 |
  | employees  | 9 |
  | checksIn   | 35 |
  | checksOut  | 45 |
