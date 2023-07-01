default: build

build-dev-company:
	'$(CURDIR)/scripts/build/dev/company.sh'

deploy-dev-company:
	'$(CURDIR)/scripts/deploy/dev/company.sh'


.PHONY: default build-dev-company deploy-dev-company