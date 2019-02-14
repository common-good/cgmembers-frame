ALTER TABLE `users` 
  DROP `changesX`, 
  DROP `share`, 
  DROP `rebate`,
  ADD `backing` DECIMAL(11,2) NOT NULL DEFAULT '0' COMMENT 'amount account-holder chose to back' AFTER `crumbs`, 
  ADD `backingDate` INT(11) NOT NULL DEFAULT '0' COMMENT 'date account-holder started backing' AFTER `backing`,
  ADD `food` DECIMAL(6,3) NOT NULL DEFAULT '0' COMMENT 'percentage of each food purchase to donate to the food fund' AFTER `backingDate`;
  
ALTER TABLE `x_users` 
  DROP `changesX`, 
  DROP `share`, 
  DROP `rebate`,
  ADD `backing` DECIMAL(11,2) NOT NULL DEFAULT '0' COMMENT 'amount account-holder chose to back' AFTER `crumbs`, 
  ADD `backingDate` INT(11) NOT NULL DEFAULT '0' COMMENT 'date account-holder started backing' AFTER `backing`,
  ADD `food` DECIMAL(6,3) NOT NULL DEFAULT '0' COMMENT 'percentage of each food purchase to donate to the food fund' AFTER `backingDate`;

ALTER TABLE `r_txs`
  DROP `payerReward`,
  DROP `payeeReward`;
  
ALTER TABLE `x_txs`
  DROP `payerReward`,
  DROP `payeeReward`;