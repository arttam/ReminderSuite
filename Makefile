all:
	$(MAKE) -C ClientLib
	$(MAKE) -C DataProvider
	$(MAKE) -C HttpWrapper
	$(MAKE) -C Server
