CREATE TABLE IF NOT EXISTS `r_industries` (
  `iid` int(11) NOT NULL AUTO_INCREMENT,
  `industry` varchar(255) COLLATE utf8_unicode_ci NOT NULL,
  `parent` int(11) NULL,
  PRIMARY KEY (`iid`),
  KEY (`parent` )
) ENGINE=InnoDB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;
