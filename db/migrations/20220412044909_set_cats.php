<?php

use Phinx\Migration\AbstractMigration;
use Phinx\Db\Adapter\MysqlAdapter as phx;

require_once __DIR__ . '/util.inc';

class SetCats extends AbstractMigration {
  public function change() {
    $t = $this->table('tx_cats');
    $t->removeColumn('type');
    
    if ($this->isMigratingUp()) {
      $this->doSql('TRUNCATE sessions'); // because w\svar() now serializes everything
      if (!$this->fetchRow('SELECT 1 FROM tx_cats WHERE id>=100')) { // don't overwrite existing data
        $this->doSql('TRUNCATE tx_cats');
        $t->insert($this->data());
      }
    }
    
    $t->update();
  }

  public function doSql($sql) {cgpr("$sql\n"); $this->execute($sql);} // uses $this, so can't be in util.inc

  private function data() {
    $flds = ray('id category description externalId');
    $rows = [
      [100,'I: Billable Expense Income','',265],
      [200,'I: Consulting and Services','Consulting Income',155],
      [300,'I: Donations','',156],
      [400,'I: Donations: Ads','',157],
      [500,'I: Donations: Company Donations','',233],
      [600,'I: Donations: Crumbs Donations','percentage of payments received through Common Good',280],
      [700,'I: Donations: Disqualified Contributions','generally this means contributions by Directors of CG and their close family members.',205],
      [800,'I: Donations: Grants','',159],
      [900,'I: Donations: One-time Donations','usually from non-members (always, before FY2019)',234],
      [1000,'I: Donations: Regular Donations','recurring monthly, quarterly, or yearly (until FY2019 included ALL member donations)',196],
      [1100,'I: Donations: Roundup Donations','automatic contribution of rounded up payment change',279],
      [1200,'I: Donations: Sponsored Donations','passed through to an organization we fiscally sponsor',318],
      [1300,'I: Donations: Sponsored Donations: Fiscal Sponsorship Fees','part of Sponsored Donations not to be spent on program costs',322],
      [1400,'I: Donations: Stepup Donations and Tips','',311],
      [1500,'I: Donations: Substantial Contributions ($5000+)','total for people giving $5,000+/FY (ignore gifts under $1)--includes "unusual grants"',206],
      [1600,'I: Donations: Substantial Contributions ($5000+): Unusual Grants','substantial gifts from "disinterested persons"',160],
      [1700,'I: Gross Sales','Gross Sales',161],
      [1800,'I: Income Uncertainty','Rewards, Inflation Adjustment, and Shared Rewards',222],
      [1900,'I: Investment Income','',295],
      [2000,'I: Investment Income: Bank Interest','Bank Interest',244],
      [2100,'I: Investment Income: Program-related Investment Income','Investment Income in keeping with our mission',162],
      [2200,'I: Markup','',258],
      [2300,'I: Sales of Product Income','',262],
      [2400,'I: Uncategorized Income','Income not categorized elsewh',188],
      [2500,'E: Ask William','',267],
      [2600,'E: Bad Debt','Bad Debt Expense',164],
      [2700,'E: Change in Risk Assessment','Change in expected value of investments or contingent grants (in or out)',293],
      [2800,'E: Cost of Goods Sold','',263],
      [2900,'E: Depreciation','',281],
      [3000,'E: Depreciation: Depreciation - FY2014 Furniture','',286],
      [3100,'E: Depreciation: Depreciation - FY2016 Equipment','',283],
      [3200,'E: Depreciation: Depreciation - FY2017 Equipment','',284],
      [3300,'E: Depreciation: Depreciation - FY2018 Equipment','',285],
      [3400,'E: Depreciation: Depreciation FY2014 - Equipment','',282],
      [3500,'E: Equipment','',165],
      [3600,'E: Equipment: CG POS equipment','',202],
      [3700,'E: Equipment: Computer Hardware & Software','',190],
      [3800,'E: Equipment: Repairs','Repairs',182],
      [3900,'E: Equipment: Resources (Books, etc.)','',183],
      [4000,'E: Event Costs','Food, lodging, equipment etc.',166],
      [4100,'E: Event Costs: Consumables','',207],
      [4200,'E: Event Costs: Equipment Rental','',246],
      [4300,'E: Event Costs: Event Fees & Space Rental','',169],
      [4400,'E: Fees','',167],
      [4500,'E: Fees: Government Fees','',170],
      [4600,'E: Fees: Interest Expense','Interest Expense',174],
      [4700,'E: Fees: Legal Fees','',171],
      [4800,'E: Fees: Tax Penalties and Interest','',240],
      [4900,'E: Fees: Transaction Fees (bank, CC, wire, etc)','',168],
      [5000,'E: Fees: Transaction Fees (bank, CC, wire, etc): Reimbursement of Transaction Fees','mostly from sponsored organizations',323],
      [5100,'E: Grants','',172],
      [5200,'E: Information Services','Dues and Subscription Expense',173],
      [5300,'E: Information Services: Websites','',185],
      [5400,'E: Marketing - Advanced','Promotional Expenses',198],
      [5500,'E: Marketing - Advanced: Entertainment','Entertainment',256],
      [5600,'E: Marketing - Advanced: Networking Fees and Dues','',253],
      [5700,'E: Marketing - Simple','',230],
      [5800,'E: Marketing - Simple: Advertising','Advertising',251],
      [5900,'E: Marketing - Simple: Member Support','',303],
      [6000,'E: Marketing - Simple: Postage / Shipping','Postage and Delivery Expense',255],
      [6100,'E: Marketing - Simple: Printing','Printing and Repro. Expense',248],
      [6200,'E: Marketing - Simple: Promotional Materials','',245],
      [6300,'E: Miscellaneous','Miscellaneous',175],
      [6400,'E: Miscellaneous: CG Account Adjustments','',203],
      [6500,'E: Miscellaneous: Reconciliation Discrepancies','Discrepancies between bank statements and company records',226],
      [6600,'E: Office','',176],
      [6700,'E: Office: Communication Services','internet, phone, zoom, otter.ai, etc.',242],
      [6800,'E: Office: Insurance','Insurance',252],
      [6900,'E: Office: Rent','',189],
      [7000,'E: Office: Supplies','Supplies',177],
      [7100,'E: Office: Utilities','Water, Gas, Electric',178],
      [7200,'E: Opening Balance Equity','Opening balances during setup',145],
      [7300,'E: Payroll Expenses','Payroll expenses',209],
      [7400,'E: Payroll Expenses: Payroll Fee','',232],
      [7500,'E: Payroll Expenses: Payroll Taxes','',237],
      [7600,'E: Payroll Expenses: Payroll Wages','',239],
      [7700,'E: Professional Fees','',179],
      [7800,'E: Professional Fees: Accounting Services','',201],
      [7900,'E: Professional Fees: Ad Hoc Consultants','',180],
      [8000,'E: Professional Fees: Computer Services','',220],
      [8100,'E: Professional Fees: Honoraria','appreciative discretionary compensation for miscellaneous services -- includes "thank you" gifts to',219],
      [8200,'E: Professional Fees: Staff Consultants','Professional Fees',181],
      [8300,'E: Professional Fees: Staff Development','',224],
      [8400,'E: Purchases','',261],
      [8500,'E: Reconciliation Discrepancies','',266],
      [8600,'E: Reconciliation Discrepancies-1','',296],
      [8700,'E: Retained Earnings','Undistributed earnings of the',144],
      [8800,'E: Sponsored Project Expenses','',314],
      [8900,'E: Sponsored Project Expenses: Grants and Direct Support to Idividuals','Direct support to eligible members for food costs',316],
      [9000,'E: Sponsored Project Expenses: Occupancy','rent or acquisition of real estate',325],
      [9100,'E: Sponsored Project Expenses: Salaries and Wages','',317],
      [9200,'E: Travel','Car & Truck',184],
      [9300,'E: Unapplied Cash Bill Payment Expense','',297],
      [9400,'E: Uncategorized Expense','',259],
      [9500,'E: _Accrued Int','Accrued Interest',186],
      [9600,'E: _IntExp','Investment Interest Exp',187],
      [9700,'A: *Accounts Receivable','Unpaid or unapplied customer',193],
      [9800,'A: 457 Escrow Asset','',290],
      [9900,'A: 457 Escrow Asset: 457 Investments','money is in shared capital cooperative',302],
      [10000,'A: Accounts Receivable','',152],
      [10100,'A: CC Processor','credit card processor, like PayPal',150],
      [10200,'A: CG Account ..AAB','Common Good\'s own Common Good Credits Account',195],
      [10300,'A: Inventory Asset','',264],
      [10400,'A: Investments','',211],
      [10500,'A: Investments: Artisan Beverage Coop Investment','',277],
      [10600,'A: Investments: Boston Community Loan Fund','',273],
      [10700,'A: Investments: CG Western MA Region Investments','investments / loans made by the "Region"',312],
      [10800,'A: Investments: Co-op Power Investment','',278],
      [10900,'A: Investments: Equity Trust Investment','',274],
      [11000,'A: Investments: NH Community Loan Fund','',269],
      [11100,'A: Investments: Northeast Biodiesel Investment','',268],
      [11200,'A: Investments: PVGrows Investment','',275],
      [11300,'A: Investments: x Loan Loss Reserve - CG Western MA','',313],
      [11400,'A: Investments: x Loan Loss Reserve - Community Funds (5%)','',288],
      [11500,'A: Investments: x Loan Loss Reserve - Northeast Biodiesel (50%)','',287],
      [11600,'A: Office Equipment','major equipment such as computers (depreciate at 20% per year)',214],
      [11700,'A: Office Furniture','',212],
      [11800,'A: Old Checking ..2275','at Citizens Bank. Old corporate account #909302151 at Greenfield Cooperative Bank. Closed October 31',146],
      [11900,'A: Operations ..8571','Brattleboro S&L account #500718571',307],
      [12000,'A: Petty Cash','',151],
      [12100,'A: Sponsored','Projects sponsored by CG',319],
      [12200,'A: Sponsored: CG Western MA Region ..AAA','The server\'s "regional" account',310],
      [12300,'A: Sponsored: Dollar Pool ..8598','Brattleboro S&L account #500718598',306],
      [12400,'A: Sponsored: Dollar Pool ..8598: MSB Escrow for Dollar Pool','BS&L asked to hold $50k in escrow',327],
      [12500,'A: Sponsored: Food Fund ..AZV','Common Good\'s fund for food subsidies',309],
      [12600,'A: Sponsored: Kibilio ..BTY','FBO Kibilio (fiscal sponsorship)',320],
      [12700,'A: Sponsored: RJ Brooklyn ..AUN','FBO Racial Justice Brooklyn (fiscal sponsorship)',321],
      [12800,'A: Uncategorized Asset','',260],
      [12900,'L: 457 Deferred Retirement Pay','',299],
      [13000,'L: 457 Deferred Retirement Pay: 457 Deferred Retirement Pay - WS','',218],
      [13100,'L: Accounts Payable','Unpaid or unapplied vendor bills or credits',231],
      [13200,'L: Capital One CC','',153],
      [13300,'L: Community Funding Guarantees FY2017-18 (10%)','We promised Common Good Greenfield, to back their first two year\'s funding ($10 + $18k).',289],
      [13400,'L: Contingent Compensation','compensation CGF owes to some former contractors',192],
      [13500,'L: Dollar Pool Liability','Amt in CG Dollar Pool we owe to CG Communities (their members, collectively) -- net member transfers',204],
      [13600,'L: EIDL Advance','',305],
      [13700,'L: Equipment Deposits','',225],
      [13800,'L: Long Term Loans','',229],
      [13900,'L: Long Term Loans: Advance Investments','for CGBank Project',243],
      [14000,'L: Long Term Loans: CG Greenfield Loan for Co-op Power','',292],
      [14100,'L: Long Term Loans: Forgivable Loans','',254],
      [14200,'L: Long Term Loans: Loan from Sally Willoughby','at 2.5% ($250 due 2/1/2020, $10,250 due 2/1/2021)',300],
      [14300,'L: Long Term Loans: S2BE checking accounts','money owed to participants',250],
      [14400,'L: Negative Balance Risk of Non-Repayment (10%)','How much of the total of all negative balances would people fail to repay.',308],
      [14500,'L: Payroll Liabilities','Unpaid payroll liabilities. Amounts withheld or accrued, but not yet paid',210],
      [14600,'L: Payroll Liabilities: 941 Taxes Liability','',236],
      [14700,'L: Payroll Liabilities: State SUTA/UI Tax Liability','',238],
      [14800,'L: Payroll Liabilities: State Withholding Tax Liability','',217],
      [14900,'L: Payroll Liabilities: To Be Paid In CG Credits','',215],
    ];
    foreach ($rows as $row) $data[] = array_combine($flds, $row);
    return $data;
  }
}