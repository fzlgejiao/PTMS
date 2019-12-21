#include "ireader.h"
#include "itag.h"
#include "irdm.h"

quint64 bytes2longlong(QByteArray& bytes)
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
void callback(TMR_Reader *rp, const TMR_TagReadData *t, void *cookie);
void exceptionCallback(TMR_Reader *rp, TMR_Status error, void *cookie);
TMR_ReadPlan	plan;
TMR_TagFilter	filter;
TMR_TagOp		tagop;
TMR_ReadListenerBlock rlb;
TMR_ReadExceptionListenerBlock reb;

iReader::iReader(QObject *parent)
	: QObject(parent)
{
	RDM = (iRDM *)parent;
	tmrReader = new TMR_Reader();
	bCreated = false;
	cur_plan = 0;
	tPlan = PLAN_NONE;
	antennaCount = 1;		//default use antenna 1
}

iReader::~iReader()
{
	stopReading();

	ret = TMR_removeReadListener(tmrReader, &rlb);
	//checkerr(tmrReader, ret, "removing read listener");

	ret = TMR_removeReadExceptionListener(tmrReader, &reb);
	//checkerr(tmrReader, ret, "removing read exception listener");

	TMR_destroy(tmrReader);
}
void iReader::checkerror()
{
	//if (ret != TMR_SUCCESS)
	{
		QString errormessage = QString(TMR_strerr(tmrReader, ret));
		qDebug() << "Error reader-init : FAILED - " << errormessage;
	}
}
bool iReader::RD_init()
{	
	//check if need to create reader
	if (bCreated)
	{
		if (m_uri == RDM->RDM_comname)
			return true;
		TMR_destroy(tmrReader);
	}

	m_uri = RDM->RDM_comname;
	ret = TMR_create(tmrReader, m_uri.toStdString().c_str());
	if (ret != TMR_SUCCESS)
		return false;
	else
		bCreated = true;
	ret = TMR_connect(tmrReader);
	if (ret != TMR_SUCCESS) return false;

	//Get reader's  hardware ,software, model type
	TMR_String str;
	char string[50];
	str.value = string;
	str.max = 50;

	ret = TMR_paramGet(tmrReader, TMR_PARAM_PRODUCT_GROUP, &str);
	if (ret != TMR_SUCCESS) return false;
	group = QString(str.value);

	ret = TMR_paramGet(tmrReader, TMR_PARAM_VERSION_HARDWARE, &str);
	if (ret != TMR_SUCCESS) return false;
	hardware = QString(str.value);

	ret = TMR_paramGet(tmrReader, TMR_PARAM_VERSION_SOFTWARE, &str);
	if (ret != TMR_SUCCESS) return false;
	software = QString(str.value);

	ret = TMR_paramGet(tmrReader, TMR_PARAM_VERSION_MODEL, &str);
	if (ret != TMR_SUCCESS) return false;
	modleversion = QString(str.value);

	//set parameters to reader
	TMR_Region region = TMR_REGION_PH;
	int power = 3000;
	bool is_send_sl = true;
	int t4 = 3000;
	//bool uniquebydata = true;									//if one simple plan ,not necessary publish by data
	bool checkport = true;
	TMR_GEN2_Session session = TMR_GEN2_SESSION_S0;
	TMR_GEN2_Tari tari = TMR_GEN2_TARI_25US;
	TMR_GEN2_TagEncoding tagcoding = TMR_GEN2_MILLER_M_4;

	ret = TMR_paramSet(tmrReader, TMR_PARAM_REGION_ID, &region);
	if (ret != TMR_SUCCESS) return false;
	ret = TMR_paramSet(tmrReader, TMR_PARAM_RADIO_READPOWER, &power);
	if (ret != TMR_SUCCESS) return false;
	ret = TMR_paramSet(tmrReader, TMR_PARAM_RADIO_WRITEPOWER, &power);
	if (ret != TMR_SUCCESS) return false;
	ret = TMR_paramSet(tmrReader, TMR_PARAM_GEN2_SEND_SELECT, &is_send_sl);
	if (ret != TMR_SUCCESS) return false;
	ret = TMR_paramSet(tmrReader, TMR_PARAM_GEN2_T4, &t4);
	if (ret != TMR_SUCCESS) return false;
	//ret = TMR_paramSet(tmrReader, TMR_PARAM_TAGREADDATA_UNIQUEBYDATA, &uniquebydata);
	//if (ret != TMR_SUCCESS) return false;
	ret = TMR_paramSet(tmrReader, TMR_PARAM_ANTENNA_CHECKPORT, &checkport);
	if (ret != TMR_SUCCESS) return false;
	ret = TMR_paramSet(tmrReader, TMR_PARAM_GEN2_SESSION, &session);
	if (ret != TMR_SUCCESS) return false;
	//ret = TMR_paramSet(tmrReader, TMR_PARAM_GEN2_TARI, &tari);
	//if (ret != TMR_SUCCESS) return false;		
	//ret = TMR_paramSet(tmrReader, TMR_PARAM_GEN2_TAGENCODING, &tagcoding);
	//if (ret != TMR_SUCCESS) return false;	

		
	//Auto check connected antennas
	antennaCount = 2;		//M6E micro support 2 antenna
	antennaList[0] = 1;
	antennaList[1] = 2;

/*
	TMR_uint8List connectedantennalist;
	connectedantennalist.list = antennalist;
	connectedantennalist.max = sizeof(antennalist);
	connectedantennalist.len = 0;

	ret = TMR_paramGet(tmrReader, TMR_PARAM_ANTENNA_CONNECTEDPORTLIST, &connectedantennalist);
	if (ret != TMR_SUCCESS) return false;

	//update antennas
	if (connectedantennalist.len == 0) return false;
	antennaCount = connectedantennalist.len;
	memcpy(antennaList, connectedantennalist.list, antennaCount);
*/
	//set OP antenna,use the first one
	antennaCount = 1;
	ret = TMR_paramSet(tmrReader, TMR_PARAM_TAGOP_ANTENNA, &antennaList[0]);
	if (ret != TMR_SUCCESS) return false;


	for (int i = 0; i<PLAN_CNT; i++)
	{
		ret = TMR_RP_init_simple(&subplan[i], antennaCount, antennaList, TMR_TAG_PROTOCOL_GEN2, 0);
		if (ret != TMR_SUCCESS) return false;
		subplanPtrs[i] = &subplan[i];
	}

	int subplanindex = 0;

	//plan to Read On-chip RSSI
	{

		quint8	OC_rssi_mask = 0x1F;
		ret = TMR_TF_init_gen2_select(&rssiFilter, false, TMR_GEN2_BANK_USER, 0xD0, 8, &OC_rssi_mask);
		if (ret != TMR_SUCCESS) return false;
		ret = TMR_TagOp_init_GEN2_ReadData(&rssiOP, TMR_GEN2_BANK_RESERVED, 0xD, 1);
		if (ret != TMR_SUCCESS) return false;
		ret = TMR_RP_set_filter(&subplan[subplanindex], &rssiFilter);
		if (ret != TMR_SUCCESS) return false;
		ret = TMR_RP_set_tagop(&subplan[subplanindex], &rssiOP);
		if (ret != TMR_SUCCESS) return false;
		subplanindex++;
	}

	//plan to Read tag temperature	
	{

		ret = TMR_TF_init_gen2_select(&tempFilter, false, TMR_GEN2_BANK_USER, 0xE0, 0, 0);
		if (ret != TMR_SUCCESS) return false;
		ret = TMR_TagOp_init_GEN2_ReadData(&tempOP, TMR_GEN2_BANK_RESERVED, 0x0E, 1);
		if (ret != TMR_SUCCESS) return false;
		ret = TMR_RP_set_filter(&subplan[subplanindex], &tempFilter);
		if (ret != TMR_SUCCESS) return false;
		ret = TMR_RP_set_tagop(&subplan[subplanindex], &tempOP);
		if (ret != TMR_SUCCESS) return false;
		subplanindex++;
	}

	//ret = TMR_RP_init_multi(&multiplan, subplanPtrs, PLAN_CNT, 0);
	//if (ret != TMR_SUCCESS) return false;
	//ret = TMR_paramSet(tmrReader, TMR_PARAM_READ_PLAN, &subplan[0]);
	//if (ret != TMR_SUCCESS) return false;

	//rlb.listener = &callback;
	//rlb.cookie = NULL;
	//reb.listener = &exceptionCallback;
	//reb.cookie = NULL;

	//ret = TMR_addReadListener(tmrReader, &rlb);
	//ret = TMR_addReadExceptionListener(tmrReader, &reb);
	//ret = TMR_startReading(tmrReader);

		//set callback
	rlb.listener = callback;
	rlb.cookie = NULL;

	reb.listener = exceptionCallback;
	reb.cookie = NULL;
	ret = TMR_addReadListener(tmrReader, &rlb);
	ret = TMR_addReadExceptionListener(tmrReader, &reb);
	return true;
}
bool iReader::wirteEpc(const QByteArray& epc_old, const QString& epc_new)
{
	TMR_TagData		oldepc;
	TMR_TagData		newepc;
	TMR_TagOp		tagop;
	TMR_TagFilter	filter;
	TMR_Status		ret;

	oldepc.epcByteCount = epc_old.count();
	memcpy(oldepc.epc, epc_old.data(), epc_old.count());

	newepc.epcByteCount = epc_new.count();
	memcpy(newepc.epc, epc_new.toStdString().c_str(), epc_new.count());

	ret = TMR_TagOp_init_GEN2_WriteTag(&tagop, &newepc);
	if (ret != TMR_SUCCESS) 
		return false;
	ret = TMR_TF_init_tag(&filter, &oldepc);
	if (ret != TMR_SUCCESS) 
		return false;

	/* Execute the tag operation Gen2 writeTag with select filter applied*/
	ret = TMR_executeTagOp(tmrReader, &tagop, &filter, NULL);
	if (ret == TMR_SUCCESS) 
		return true;
	else
	{
		qDebug() << "Error reader-wirteEpc : FAILED - " << QString(TMR_strerr(tmrReader, ret));
		return false;
	}
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
	quint8 b0 = byts[0];		//Epc TID first byte may be 0xE0 or 0xE2 in Gen2

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
	
	//clear old online tags before read again
	RDM->tagOnline.clear();
	if (!switchplans()) return;


	//--------------------------test code---------------------
	//quint64 tid1 = 1;
	//for (uchar i = 0; i < 16; i++)
	//{
	//	tid1 = i;
	//	QByteArray epc1;
	//	epc1.append('1'+i);
	//	RDM->tagOnline.insert(tid1, epc1);
	//}
	//--------------------------test code---------------------
	int readCount = 0;
	ret = TMR_read(tmrReader, RD_TIMEOUT, &readCount);
	if (ret != TMR_SUCCESS)
	{
		QString errormessage = QString(TMR_strerr(tmrReader, ret));
		qDebug() << "Error reader-readtag : FAILED - " << errormessage;
		return;
	}
	qDebug() << "Info reader-readtag : SUCCESS	ReadCounts=" << readCount;

	while (TMR_SUCCESS == TMR_hasMoreTags(tmrReader))
	{
		TMR_TagReadData trd;
		//prepare data buff
		//quint8 dataBuff[256];
		quint8 databuffer[4];
		//ret = TMR_TRD_init_data(&trd, sizeof(dataBuff), dataBuff);		
		
		trd.data.max = sizeof(databuffer);
		trd.data.list = databuffer;
		trd.data.len = 0;

		ret = TMR_getNextTag(tmrReader, &trd);
		if (ret != TMR_SUCCESS) 
			continue;

		//read tag ok!
		TMR_TagFilter epcfilter;
		ret = TMR_TF_init_tag(&epcfilter, &trd.tag);
		if (ret != TMR_SUCCESS)
			continue;
			
		if (trd.data.len > 0)
		{
			//get Tid ,epc		
			quint64 tid = readtagTid(&epcfilter);
			if (tid == 0) 
				continue;
			QByteArray tEPC((char *)trd.tag.epc, trd.tag.epcByteCount);
			
			//add tag into online list
			RDM->tagOnline.insert(tid, tEPC);

			iTag * tag = RDM->Tag_get(tid);
			if (tag )
			{
				tag->T_ticks = TAG_TICKS;
				tag->T_alarm_offline = false;
				tag->T_epc = tEPC;
				tag->T_rssi = trd.rssi;
				tag->T_data_flag |= Tag_Online; 
				if (tag->T_caldata.all == 0)
					tag->T_caldata.all = readtagCalibration(&epcfilter);

				quint8 data0 = trd.data.list[0];	//use the data0 check the OC-Rssi plan or temperature plan
				if (data0 != 0)
				{
					ushort temperaturecode = (trd.data.list[0] << 8) + trd.data.list[1];
					if (temperaturecode > 0)
					{
						float Temp = tag->parseTCode(temperaturecode);
						if (tag->T_temp == 0.0														//init temperature
							|| (qAbs(Temp - tag->T_temp) < qAbs(tag->T_temp)*0.5))					//reasonable temperature
						{
							tag->T_temp = Temp;
							if (tag->T_temp > tag->T_uplimit)										//bigger than up limit
								tag->T_alarm_temperature = true;
							qDebug()<< "managed tag : sid = " << tag->T_sid
									<< " uid = " << tag->T_uid
									<< " epc = " << tag->T_epc
									<< " rssi = " << tag->T_rssi
									<< " temperature = " << tag->T_temp
									<< " temp_alarmed = " << tag->T_alarm_temperature;
						}
					}
				}
				else 
				{
					tag->T_OC_rssi = trd.data.list[1];

					qDebug()<< "managed tag : sid = " << tag->T_sid
							<< " uid = " << tag->T_uid
							<< " epc = " << tag->T_epc
							<< " rssi = " << tag->T_rssi
							<< " Oc-rssi = " << tag->T_OC_rssi
							<< "frequency =" << trd.frequency;
				}
				emit tagUpdated(tag);
			}
			else
			{
				qDebug() << "unknown tag : uid = " << tid << "epc = " << tEPC;
			}

		}//if (trd.data.len > 0)
	}//while
	qDebug() << "Online tags count: " << RDM->tagOnline.count();

}

bool iReader::switchplans()
{
	if (cur_plan == PLAN_CNT)
	{
		cur_plan = 0;
	}
	if (!tmrReader->connected) return false;
	ret = TMR_paramSet(tmrReader, TMR_PARAM_READ_PLAN, &subplan[cur_plan]);
	if (ret == TMR_SUCCESS)
	{
		cur_plan++;
		return true;
	}
	else
		return false;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

void checkerr(TMR_Reader* rp, TMR_Status ret, const char *msg)
{
	qDebug() << QString("Error %1: %2").arg(msg).arg(TMR_strerr(rp, ret));
}
void callback(TMR_Reader *rp, const TMR_TagReadData *t, void *cookie)
{
	TMR_Status  ret;
	//char epcStr[128];
	//TMR_bytesToHex(t->tag.epc, t->tag.epcByteCount, epcStr);
	QByteArray tEPC((char *)t->tag.epc, t->tag.epcByteCount);
	QString epc = tEPC;
	QDateTime datetime = QDateTime::currentDateTime();
	QString time = datetime.toString("yyyy-MM-dd   hh:mm:ss.zzz");
	qDebug() << QString("%1 - Background read: epc=%2,rssi=%3").arg(time).arg(epc).arg(t->rssi);

	TMR_TagFilter epcfilter;
	ret = TMR_TF_init_tag(&epcfilter, (TMR_TagData *)&t->tag);
	if (ret != TMR_SUCCESS) 
		return;
	
	iRDM& RDM = iRDM::Instance();
	iReader* reader = RDM.getReader();
	if (!reader)
		return;
	if (t->data.len == 0)
		return;
	if (*(int *)cookie == PLAN_CALI)
	{
		QByteArray Calibratebytes((char *)t->data.list, t->data.len);
		quint64 cali = bytes2longlong(Calibratebytes);
		qDebug() << QString("calibration : %1").arg(cali);
		reader->callbackCalibration(epc, t->rssi,cali);
	}
	else if (*(int *)cookie == PLAN_TEMP)
	{
		ushort temperaturecode = (t->data.list[0] << 8) + t->data.list[1];
		qDebug() << QString("Temp code : %1").arg(temperaturecode, 4, 16);
		reader->callbackTempCode(epc, t->rssi, temperaturecode);
	}
	else if (*(int *)cookie == PLAN_OCRSSI)
	{
		qint8 ocrssi = t->data.list[1];
		qDebug() << QString("OC-RSSI : %1").arg(ocrssi);
		reader->callbackOCRSSI(epc, t->rssi, ocrssi);
	}
/*
	char dataStr[258] = {0};
	if (0 < t->data.len)
	{
		memset(dataStr, 0, 258);
		TMR_bytesToHex(t->data.list, t->data.len, dataStr);

		qDebug() << QString("Data Bank(%1):").arg(t->data.len) << QString(dataStr);
	}
	if (0 < t->userMemData.len)
	{
		memset(dataStr, 0, 258);
		TMR_bytesToHex(t->userMemData.list, t->userMemData.len, dataStr);

		qDebug() << QString("User Bank(%1):").arg(t->userMemData.len) << QString(dataStr);
	}
	if (0 < t->epcMemData.len)
	{
		memset(dataStr, 0, 258);
		TMR_bytesToHex(t->epcMemData.list, t->epcMemData.len, dataStr);
		qDebug() << QString("epc Bank(%1):").arg(t->epcMemData.len) << QString(dataStr);
	}
	if (0 < t->reservedMemData.len)
	{
		memset(dataStr, 0, 258);
		TMR_bytesToHex(t->reservedMemData.list, t->reservedMemData.len, dataStr);
		qDebug() << QString("resv Bank(%1):").arg(t->reservedMemData.len) << QString(dataStr);
	}
	if (0 < t->tidMemData.len)
	{
		memset(dataStr, 0, 258);
		TMR_bytesToHex(t->tidMemData.list, t->tidMemData.len, dataStr);
		qDebug() << QString("tid Bank(%1):").arg(t->tidMemData.len) << QString(dataStr);

		QByteArray byts((char *)t->tidMemData.list, t->tidMemData.len);

	
		QByteArray idbytes;
		quint8 b0 = byts[0];		//Epc TID first byte may be 0xE0 or 0xE2 in Gen2

		if (b0 == 0xE2)
			idbytes = byts.right(8);
		else if (b0 == 0xE0)
			idbytes = byts.left(8);


		quint64 tid =  bytes2longlong(idbytes);
		return;
	}
*/
}

void exceptionCallback(TMR_Reader *rp, TMR_Status error, void *cookie)
{
	qDebug() << QString("Exception callback Error: %1").arg(TMR_strerr(rp, error));
}

void iReader::moveNextPlan()
{
	if (tPlan < PLAN_OCRSSI)
		tPlan = (PLAN_TYPE)(tPlan + 1);
	else
		tPlan = PLAN_CALI;
}
void iReader::startReading()
{
	uint8_t			data[258];
	TMR_Status		ret;
	TMR_uint8List	dataList;
	dataList.len = dataList.max = 258;
	dataList.list = data;
	int readCount = 0;
	if (tPlan == PLAN_NONE)
		return;
	if (tPlan == PLAN_CALI)
	{
		ret = TMR_RP_init_simple(&plan, antennaCount, antennaList, TMR_TAG_PROTOCOL_GEN2, 1000);
		checkerr(tmrReader, ret, "initializing read plan : read calibration");

		//In UserBank  ----word offset=0x8, word length=4
		ret = TMR_TagOp_init_GEN2_ReadData(&tagop, TMR_GEN2_BANK_USER, 0x8, 4);
		checkerr(tmrReader, ret, "initializing tagop");
		ret = TMR_RP_set_tagop(&plan, &tagop);
		checkerr(tmrReader, ret, "setting tagop");

		// Commit read plan 
		ret = TMR_paramSet(tmrReader, TMR_PARAM_READ_PLAN, &plan);
		checkerr(tmrReader, ret, "setting read plan");

		//set callback
		//rlb.listener = callback;
		rlb.cookie = &tPlan;

		//reb.listener = exceptionCallback;
		//reb.cookie = NULL;
	}
	else if (tPlan == PLAN_TEMP)
	{
		ret = TMR_RP_init_simple(&plan, antennaCount, antennaList, TMR_TAG_PROTOCOL_GEN2, 1000);
		checkerr(tmrReader, ret, "initializing read plan : read temperature code");

		ret = TMR_TF_init_gen2_select(&filter, false, TMR_GEN2_BANK_USER, 0xE0, 0, 0);
		checkerr(tmrReader, ret, "initializing select filter");
		ret = TMR_RP_set_filter(&plan, &filter);
		checkerr(tmrReader, ret, "setting tag filter");

		ret = TMR_TagOp_init_GEN2_ReadData(&tagop, TMR_GEN2_BANK_RESERVED, 0x0E, 1);
		checkerr(tmrReader, ret, "initializing tagop");
		ret = TMR_RP_set_tagop(&plan, &tagop);
		checkerr(tmrReader, ret, "setting tagop");

		// Commit read plan 
		ret = TMR_paramSet(tmrReader, TMR_PARAM_READ_PLAN, &plan);
		checkerr(tmrReader, ret, "setting read plan");

		//set callback
		//rlb.listener = callback;
		rlb.cookie = &tPlan;

		//reb.listener = exceptionCallback;
		//reb.cookie = NULL;	
	}
	else if (tPlan == PLAN_OCRSSI)
	{
		ret = TMR_RP_init_simple(&plan, antennaCount, antennaList, TMR_TAG_PROTOCOL_GEN2, 1000);
		checkerr(tmrReader, ret, "initializing read plan : read oc-rssi");

		quint8	OC_rssi_mask = 0x1F;
		ret = TMR_TF_init_gen2_select(&filter, false, TMR_GEN2_BANK_USER, 0xD0, 8, &OC_rssi_mask);
		checkerr(tmrReader, ret, "initializing select filter");
		ret = TMR_RP_set_filter(&plan, &filter);
		checkerr(tmrReader, ret, "setting tag filter");

		ret = TMR_TagOp_init_GEN2_ReadData(&tagop, TMR_GEN2_BANK_RESERVED, 0x0D, 1);
		checkerr(tmrReader, ret, "initializing tagop");
		ret = TMR_RP_set_tagop(&plan, &tagop);
		checkerr(tmrReader, ret, "setting tagop");

		// Commit read plan 
		ret = TMR_paramSet(tmrReader, TMR_PARAM_READ_PLAN, &plan);
		checkerr(tmrReader, ret, "setting read plan");

		//set callback
		//rlb.listener = callback;
		rlb.cookie = &tPlan;

		//reb.listener = exceptionCallback;
		//reb.cookie = NULL;
	}

	//ret = TMR_addReadListener(tmrReader, &rlb);
	//checkerr(tmrReader, ret, "adding read listener");

	//ret = TMR_addReadExceptionListener(tmrReader, &reb);
	//checkerr(tmrReader, ret, "adding exception listener");

	ret = TMR_startReading(tmrReader);
	checkerr(tmrReader, ret, "starting reading");
}
void iReader::stopReading()
{
	TMR_Status		ret;
	if (tPlan == PLAN_NONE)
		return;

	TMR_stopReading(tmrReader);

	//ret = TMR_removeReadListener(tmrReader, &rlb);
	//checkerr(tmrReader, ret, "removing read listener");

	//ret = TMR_removeReadExceptionListener(tmrReader, &reb);
	//checkerr(tmrReader, ret, "removing read exception listener");
}
void iReader::callbackCalibration(const QString& epc, qint32 rssi, quint64 calibration)
{
	qDebug() << "callbackCalibration............";

	//add tag into online list
	//RDM->tagOnline.insert(tid, tEPC);

	iTag * tag = RDM->Tag_getbysid(1);
	if (tag)
	{
		tag->T_ticks = TAG_TICKS;
		tag->T_alarm_offline = false;
		tag->T_caldata.all = calibration;
		//tag->T_epc = tEPC;
		tag->T_rssi = rssi;
		tag->T_data_flag |= Tag_Online;
	}
	else
	{
		qDebug() << "unknown tag : epc = " << epc;
	}
}
void iReader::callbackTempCode(const QString& epc, qint32 rssi, ushort tempCode)
{
	qDebug() << "callbackTempCode............";

	//add tag into online list
	//RDM->tagOnline.insert(tid, tEPC);

	iTag * tag = RDM->Tag_getbysid(1);
	if (tag)
	{
		tag->T_ticks = TAG_TICKS;
		tag->T_alarm_offline = false;
		//tag->T_epc = tEPC;
		tag->T_rssi = rssi;
		tag->T_data_flag |= Tag_Online;

		if (tempCode > 0 && tag->T_caldata.all != 0)
		{
			float Temp = tag->parseTCode(tempCode);
			if (tag->T_temp == 0.0														//init temperature
				|| (qAbs(Temp - tag->T_temp) < qAbs(tag->T_temp)*0.5))					//reasonable temperature
			{
				tag->T_temp = Temp;
				if (tag->T_temp > tag->T_uplimit)										//bigger than up limit
					tag->T_alarm_temperature = true;
				qDebug() << "managed tag : sid = " << tag->T_sid
					<< " uid = " << tag->T_uid
					<< " epc = " << tag->T_epc
					<< " rssi = " << tag->T_rssi
					<< " temperature = " << tag->T_temp
					<< " temp_alarmed = " << tag->T_alarm_temperature;
			}
		}
		emit tagUpdated(tag);
	}
	else
	{
		qDebug() << "unknown tag : epc = " << epc;
	}

}
void iReader::callbackOCRSSI(const QString& epc, qint32 rssi, qint8 ocrssi)
{
	qDebug() << "callbackOCRSSI............";

	//add tag into online list
	//RDM->tagOnline.insert(tid, tEPC);

	iTag * tag = RDM->Tag_getbysid(1);
	if (tag)
	{
		tag->T_ticks = TAG_TICKS;
		tag->T_alarm_offline = false;
		//tag->T_epc = tEPC;
		tag->T_rssi = rssi;
		tag->T_OC_rssi = ocrssi;
		tag->T_data_flag |= Tag_Online;

		emit tagUpdated(tag);
	}
	else
	{
		qDebug() << "unknown tag : epc = " << epc;
	}
}