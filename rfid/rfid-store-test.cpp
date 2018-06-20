#ifndef ARDUINO

/* CPPunit Includes */
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/extensions/HelperMacros.h>

/* Application Includes */

#include "uid.h"
#include "rfid.h"
#include "rfid-store.h"

static void __fill_store_with_data()
{
	UID to_save = {10, {0, 0, 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF}};
	rfid_store_clear_all();
	for (uint8_t i=0; i<RFID_SLOT_COUNT; i++)
	{
		to_save.bytes[0] = i;
		for (uint8_t j=0; j<RFID_HISTORY_COUNT; j++)
		{
			to_save.bytes[1] = j;
			rfid_store_save_uid(to_save, i);
		}
	}
}

/* RFID non volatile mocks */
static UID s_uid_mock[RFID_SLOT_COUNT*RFID_HISTORY_COUNT];
void rfid_nv_setup();
void rfid_nv_get_uid(UID& uid, uint8_t direct_index) { uid = s_uid_mock[direct_index]; }
void rfid_nv_set_uid(UID& uid, uint8_t direct_index) { s_uid_mock[direct_index] = uid; }
void rfid_nv_clear_uid(uint8_t direct_index) { s_uid_mock[direct_index].size = 0; }

/* END RFID non volatile mocks */

class RFIDStoreTestFixture : public CppUnit::TestFixture
{ 
	CPPUNIT_TEST_SUITE( RFIDStoreTestFixture );
	CPPUNIT_TEST( test_clear_rfid );
	CPPUNIT_TEST( test_save_rfid );
	CPPUNIT_TEST( test_get_rfid );
	CPPUNIT_TEST( test_match_rfid );
	CPPUNIT_TEST( test_save_rfid_overwrites_oldest_uid );
	CPPUNIT_TEST_SUITE_END();
public:
	
	void test_clear_rfid()
	{
		UID uid;
		__fill_store_with_data();
		for (uint8_t i=0; i<RFID_SLOT_COUNT; i++)
		{
			rfid_store_clear_slot_history(i);
			for (uint8_t j=0; j<RFID_HISTORY_COUNT; j++)
			{
				CPPUNIT_ASSERT(rfid_store_get_uid(uid, i, j) == false);
			}
		}
	}

	void test_save_rfid()
	{
		UID to_save = {10, {0,1,2,3,4,5,6,7,8,9}};
		rfid_store_clear_all();
		rfid_store_save_uid(to_save, 0);
		CPPUNIT_ASSERT(rfid_store_check_uid(0,0));
	}

	void test_get_rfid()
	{
		UID uid;
		__fill_store_with_data();
		for (uint8_t i=0; i<RFID_SLOT_COUNT; i++)
		{
			for (uint8_t j=0; j<RFID_HISTORY_COUNT; j++)
			{
				CPPUNIT_ASSERT(rfid_store_get_uid(uid, i, j));
				CPPUNIT_ASSERT_EQUAL((uint8_t)10, uid.size);
				CPPUNIT_ASSERT_EQUAL(i, uid.bytes[0]);
				CPPUNIT_ASSERT_EQUAL(j, uid.bytes[1]);
			}
		}
	}

	void test_match_rfid()
	{
		uint8_t match_slot, match_index;
		__fill_store_with_data();
		UID to_match = {10, {0, 0, 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF}};
		for (uint8_t i=0; i<RFID_SLOT_COUNT; i++)
		{
			to_match.bytes[0] = i;
			for (uint8_t j=0; j<RFID_HISTORY_COUNT; j++)
			{
				to_match.bytes[1] = j;
				CPPUNIT_ASSERT(rfid_store_match_saved(to_match));
				CPPUNIT_ASSERT(rfid_store_match_in_slot(to_match, i));
				CPPUNIT_ASSERT(rfid_store_match_saved(to_match, match_slot, match_index));
				CPPUNIT_ASSERT_EQUAL(i, match_slot);
				CPPUNIT_ASSERT_EQUAL(j, match_index);
			}
		}
	}

	void test_save_rfid_overwrites_oldest_uid()
	{
		UID to_save = {10, {10,11,12,13,14,15,16,17,18,19}};
		UID actual;
		__fill_store_with_data();

		rfid_store_save_uid(to_save, 0);

		rfid_store_get_uid(actual, 0, 0);
		CPPUNIT_ASSERT_EQUAL((uint8_t)10, actual.size);
		CPPUNIT_ASSERT_EQUAL((uint8_t)0, actual.bytes[0]);
		CPPUNIT_ASSERT_EQUAL((uint8_t)1, actual.bytes[1]);

		rfid_store_get_uid(actual, 0, 2);
		CPPUNIT_ASSERT_EQUAL((uint8_t)10, actual.size);
		for (uint8_t i=0; i<10; i++)
		{
			CPPUNIT_ASSERT_EQUAL((uint8_t)(i+10), actual.bytes[i]);
		}
	}
};

int main( int argc, char* argv[] )
{
	(void)argc; (void)argv;
	CppUnit::TextUi::TestRunner runner;
   
	CPPUNIT_TEST_SUITE_REGISTRATION( RFIDStoreTestFixture );

	CppUnit::TestFactoryRegistry &registry = CppUnit::TestFactoryRegistry::getRegistry();

	runner.addTest( registry.makeTest() );
	runner.run();
	return 0;
}

#endif
