#include "ireader.h"
#include "itag.h"
#include "irdm.h"

iReader::iReader(QString comName, QObject *parent)
	: QObject(parent)
{
	RDM = (iRDM *)parent;
	m_uri = comName;

	TMR_Status ret;
	tmrReader = new TMR_Reader();
	ret = TMR_create(tmrReader, m_uri.toStdString().c_str());

	if (ret != TMR_SUCCESS) return;
	ret = TMR_connect(tmrReader);
	if (ret != TMR_SUCCESS) return;

	//Get reader's  hardware ,software, modle type
	TMR_String str;
	char string[50];
	str.value = string;
	str.max = 50;

	ret = TMR_paramGet(tmrReader, TMR_PARAM_PRODUCT_GROUP, &str);
	if (ret != TMR_SUCCESS) return;
	group = QString(str.value);
	str.value = string;
	ret = TMR_paramGet(tmrReader, TMR_PARAM_VERSION_HARDWARE, &str);
	if (ret != TMR_SUCCESS) return;
	hardware = QString(str.value);
	str.value = string;
	ret = TMR_paramGet(tmrReader, TMR_PARAM_VERSION_SOFTWARE, &str);
	if (ret != TMR_SUCCESS) return;
	software = QString(str.value);
	str.value = string;
	ret = TMR_paramGet(tmrReader, TMR_PARAM_VERSION_MODEL, &str);
	if (ret != TMR_SUCCESS) return;
	modleversion = QString(str.value);
	str.value = string;

	TMR_Region region = TMR_REGION_PRC;
	int power = 3000;
	int t4 = 3000;

	ret = TMR_paramSet(tmrReader, TMR_PARAM_REGION_ID, &region);
	if (ret != TMR_SUCCESS) return;
	ret = TMR_paramSet(tmrReader, TMR_PARAM_RADIO_READPOWER, &power);
	if (ret != TMR_SUCCESS) return;
	ret = TMR_paramSet(tmrReader, TMR_PARAM_GEN2_T4, &t4);
	if (ret != TMR_SUCCESS) return;
	//To do :set simple plan ,read tag temperature	
	ret = TMR_TF_init_gen2_select(&tempselect, false, TMR_GEN2_BANK_USER, 0xE0, 0, 0);
	if (ret != TMR_SUCCESS) return;
	ret = TMR_TagOp_init_GEN2_ReadData(&tempread, TMR_GEN2_BANK_RESERVED, 0xE, 1);
	if (ret != TMR_SUCCESS) return;

	quint8 antennaCount = 2;		//M6E micro support 2 antenna
	antennaList[0] = 1;
	antennaList[1] = 2;

	ret = TMR_RP_init_simple(&plan, antennaCount, antennaList, TMR_TAG_PROTOCOL_GEN2, 1000);
	if (ret != TMR_SUCCESS) return;

	ret = TMR_RP_set_filter(&plan, &tempselect);
	if (ret != TMR_SUCCESS) return;
	ret = TMR_RP_set_tagop(&plan, &tempread);
	if (ret != TMR_SUCCESS) return;
	ret = TMR_paramSet(tmrReader, TMR_PARAM_READ_PLAN, &plan);
	if (ret != TMR_SUCCESS) return;

	//set async read mode	
	//setCReader();
	//TMR_ReadListener ll = std::bind(readcallback)

	//TMR_ReadExceptionListener exceptionlistner = exceptioncallback;


	//func = (myreadcallback)&readcallback;

	//rlb.listener = CReader::readcallback;
	//rlb.cookie = NULL;
	//reb.listener = &exceptioncallback;
	//reb.cookie = NULL;

	//ret = TMR_addReadListener(reader, &rlb);
	//if (ret != TMR_SUCCESS) return;
	//ret = TMR_addReadExceptionListener(reader, &reb);
	//if (ret != TMR_SUCCESS) return;
	//ret = TMR_startReading(reader);
	//if (ret != TMR_SUCCESS) return;
}

iReader::~iReader()
{
	TMR_destroy(tmrReader);
}
bool iReader::wirteEpc(iTag *tag, QString epcstr)
{
	TMR_TagData		epc;
	TMR_TagData		newepc;
	TMR_TagOp		tagop;
	TMR_TagFilter	filter;
	TMR_Status		ret;

	epc.epcByteCount = tag->T_epc.length();
	memcpy(epc.epc, tag->T_epc.toStdString().c_str(), epc.epcByteCount);

	newepc.epcByteCount = epcstr.length();
	memcpy(newepc.epc, epcstr.toStdString().c_str(), newepc.epcByteCount);

	ret = TMR_TagOp_init_GEN2_WriteTag(&tagop, &newepc);
	if (ret != TMR_SUCCESS) return false;
	ret = TMR_TF_init_tag(&filter, &epc);
	if (ret != TMR_SUCCESS) return false;
	/* Execute the tag operation Gen2 writeTag with select filter applied*/
	ret = TMR_executeTagOp(tmrReader, &tagop, &filter, NULL);

	if (ret == TMR_SUCCESS) return true;
	else return false;
}

quint64 iReader::readtagTid(TMR_TagFilter *filter)
{
	TMR_TagOp		readTid;
	TMR_uint8List	tiddata;
	TMR_Status		ret;
	quint8			buff[32];

	tiddata.max = sizeof(buff);
	tiddata.list = buff;
	tiddata.len = 0;
	//read all TID BANK
	ret = TMR_TagOp_init_GEN2_ReadData(&readTid, TMR_GEN2_BANK_TID, 0, 0);
	if (ret != TMR_SUCCESS) return 0;
	ret = TMR_executeTagOp(tmrReader, &readTid, filter, &tiddata);
	if (ret != TMR_SUCCESS) return 0;
	QByteArray byts((char *)tiddata.list, tiddata.len);

	if (tiddata.len < 8) return 0;

	QByteArray idbytes;
	byte b0 = byts[0];		//Epc TID first byte may be 0xE0 or 0xE2 in Gen2

	if (b0 == 0xE2)
		idbytes = byts.right(8);
	else if (b0 == 0xE0)
		idbytes = byts.left(8);
	else return 0;

	return bytes2longlong(idbytes);
}
quint64 iReader::readtagCalibration(TMR_TagFilter *filter)
{
	TMR_TagOp		readcalibration;
	TMR_uint8List	calibrationdata;
	TMR_Status		ret;
	quint8 buff[32];

	calibrationdata.max = sizeof(buff);
	calibrationdata.list = buff;
	calibrationdata.len = 0;
	//In UserBank  ----word offset=0x8, word length=4
	ret = TMR_TagOp_init_GEN2_ReadData(&readcalibration, TMR_GEN2_BANK_USER, 0x8, 4);
	if (ret != TMR_SUCCESS) return 0;
	ret = TMR_executeTagOp(tmrReader, &readcalibration, filter, &calibrationdata);
	if (ret != TMR_SUCCESS) return 0;

	QByteArray Calibratebytes((char *)calibrationdata.list, calibrationdata.len);

	return bytes2longlong(Calibratebytes);
}
void iReader::readtag()
{
	TMR_Status		ret;

	ret = TMR_read(tmrReader, 500, NULL);
	if (ret != TMR_SUCCESS)
	{
		QString errormessage = QString(TMR_strerr(tmrReader, ret));
		return;
	}
	while (TMR_SUCCESS == TMR_hasMoreTags(tmrReader))
	{
		TMR_TagReadData trd;
		//prepare data buff
		//quint8 dataBuff[256];
		quint8 temperatureBuff[4];
		//quint8 TidBuff[32];
		//quint8 userBuff[32];
		//ret = TMR_TRD_init_data(&trd, sizeof(dataBuff), dataBuff);		

		/*trd.tidMemData.max = sizeof(TidBuff);
		trd.tidMemData.list = TidBuff;
		trd.tidMemData.len = 0;

		trd.userMemData.max = sizeof(userBuff);
		trd.userMemData.list = userBuff;
		trd.userMemData.len = 0;*/

		trd.data.max = sizeof(temperatureBuff);
		trd.data.list = temperatureBuff;
		trd.data.len = 0;

		ret = TMR_getNextTag(tmrReader, &trd);
		if (ret != TMR_SUCCESS) continue;
		//read tag ok!
		//get Tid ,epc		
		QString epc = QString(QByteArray((char *)trd.tag.epc, trd.tag.epcByteCount));
		TMR_TagFilter epcfilter;

		ret = TMR_TF_init_tag(&epcfilter, &trd.tag);
		if (ret != TMR_SUCCESS) continue;


		/*if (trd.tidMemData.len > 0)
		{
			QByteArray tidmemdbytes((char *)trd.tidMemData.list);
			QByteArray tid = tidmemdbytes.right(8);
		}
		if (trd.userMemData.len > 0)
		{
			QByteArray calibratebytes((char *)trd.userMemData.list);
			QByteArray CaliBytes = calibratebytes.right(8);
		}*/
		if (trd.data.len > 0)
		{
			quint64 tid = readtagTid(&epcfilter);
			if (tid == 0) continue;
			
			iTag * tag = RDM->Tag_get(tid);

			if (tag )
			{
				tag->T_epc = epc;
				if (tag->T_caldata.all == 0)
					tag->T_caldata.all = readtagCalibration(&epcfilter);
				ushort temperaturecode = (trd.data.list[0] << 8) + trd.data.list[1];
				if (temperaturecode > 0)
					tag->T_temp = tag->parseTCode(temperaturecode);
			}

		}
	}
}
void iReader::readcallback(TMR_Reader *reader, const TMR_TagReadData *t, void *cookie)
{
	char epcStr[128];
	quint64 tagid;
	quint8 temperatureBuff[4];
	TMR_TagReadData hi;
	TMR_Status ret;

	ret = TMR_TRD_init_data(&hi, sizeof(temperatureBuff), temperatureBuff);

	TMR_bytesToHex(t->tag.epc, t->tag.epcByteCount, epcStr);

	float tt = 12.6f;

	//emit currenttemp(tt);
}


void iReader::exceptioncallback(TMR_Reader *reader, TMR_Status error, void *cookie)
{
	QString errormessage = QString(TMR_strerr(reader, error));
}
quint64 iReader::bytes2longlong(QByteArray& bytes)
{
	if (bytes.length() != 8) return 0;

	quint64 d0 = bytes[0] & 0xff;
	quint64 d1 = bytes[1] & 0xff;
	quint64 d2 = bytes[2] & 0xff;
	quint64 d3 = bytes[3] & 0xff;
	quint64 d4 = bytes[4] & 0xff;
	quint64 d5 = bytes[5] & 0xff;
	quint64 d6 = bytes[6] & 0xff;
	quint64 d7 = bytes[7] & 0xff;
	d0 <<= 56;
	d1 <<= 48;
	d2 <<= 40;
	d3 <<= 32;
	d4 <<= 24;
	d5 <<= 16;
	d6 <<= 8;
	return (d0 | d1 | d2 | d3 | d4 | d5 | d6 | d7);
}