ACLOCAL_AMFLAGS = -I m4
EXTRA_DIST = CMakeLists.txt INSTALL NOTICE README RELEASE database/CMakeLists.txt
SUBDIRS = doc examples src test

# Docs
#
databasedir=$(datadir)/doc/$(PACKAGE_TARNAME)-$(VERSION)/database

dist_database_DATA=$(DATABASE)

DATABASE=\
	database/Amm.dat\
	database/ColdChem.dat\
	database/core10.dat\
	database/frezchem.dat\
	database/iso.dat\
	database/llnl.dat\
	database/minteq.dat\
	database/minteq.v4.dat\
	database/phast.dat\
	database/pitzer.dat\
	database/sit.dat\
	database/Tipping_Hurley.dat\
	database/wateq4f.dat
