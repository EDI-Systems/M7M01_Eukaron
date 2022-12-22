/*====================================================================
* Project:  Board Support Package (BSP)
* Function: Handling of interrupts on TC27x
*
* Copyright HighTec EDV-Systeme GmbH 1982-2013
*====================================================================*/

#ifndef __INTERRUPTS_H__
#define __INTERRUPTS_H__

/* Interrupt SRC IDs */
#define SRC_ID_CPU0SBSRC		0	/* CPU 0 Software Breakpoint Service Request */
#define SRC_ID_CPU1SBSRC		1	/* CPU 1 Software Breakpoint Service Request */
#define SRC_ID_CPU2SBSRC		2	/* CPU 2 Software Breakpoint Service Request */

#define SRC_ID_EMEM				8	/* Emulation Memory Service Request (ED only) */
#define SRC_ID_AGBT				9	/* AGBT Service Request */

#define SRC_ID_BCUSPBSRC		16	/* Bus Control Unit SPB Service Request */

#define SRC_ID_XBARSRC			18	/* XBAR_SRI Service Request */

#define SRC_ID_CERBERUS0		20	/* Cerberus Service Request 0 */
#define SRC_ID_CERBERUS1		21	/* Cerberus Service Request 1 */

#define SRC_ID_ASCLIN0TX		32	/* ASCLIN 0 Transmit Service Request */
#define SRC_ID_ASCLIN0RX		33	/* ASCLIN 0 Receive Service Request */
#define SRC_ID_ASCLIN0EX		34	/* ASCLIN 0 Error Service Request */
#define SRC_ID_ASCLIN1TX		35	/* ASCLIN 1 Transmit Service Request */
#define SRC_ID_ASCLIN1RX		36	/* ASCLIN 1 Receive Service Request */
#define SRC_ID_ASCLIN1EX		37	/* ASCLIN 1 Error Service Request */
#define SRC_ID_ASCLIN2TX		38	/* ASCLIN 2 Transmit Service Request */
#define SRC_ID_ASCLIN2RX		39	/* ASCLIN 2 Receive Service Request */
#define SRC_ID_ASCLIN2EX		40	/* ASCLIN 2 Error Service Request */
#define SRC_ID_ASCLIN3TX		41	/* ASCLIN 3 Transmit Service Request */
#define SRC_ID_ASCLIN3RX		42	/* ASCLIN 3 Receive Service Request */
#define SRC_ID_ASCLIN3EX		43	/* ASCLIN 3 Error Service Request */

#define SRC_ID_QSPI0TX			100	/* QSPI 0 Transmit Service Request */
#define SRC_ID_QSPI0RX			101	/* QSPI 0 Receive Service Request */
#define SRC_ID_QSPI0ERR			102	/* QSPI 0 Error Service Request */
#define SRC_ID_QSPI0PT			103	/* QSPI 0 Phase Transition Service Request */
#define SRC_ID_QSPI0HC			104	/* QSPI 0 Half Cycle Service Request */
#define SRC_ID_QSPI0U			105	/* QSPI 0 User Defined Service Request */
#define SRC_ID_QSPI1TX			106	/* QSPI 1 Transmit Service Request */
#define SRC_ID_QSPI1RX			107	/* QSPI 1 Receive Service Request */
#define SRC_ID_QSPI1ERR			108	/* QSPI 1 Error Service Request */
#define SRC_ID_QSPI1PT			109	/* QSPI 1 Phase Transition Service Request */
#define SRC_ID_QSPI1HC			110	/* QSPI 1 Half Cycle Service Request */
#define SRC_ID_QSPI1U			111	/* QSPI 1 User Defined Service Request */
#define SRC_ID_QSPI2TX			112	/* QSPI 2 Transmit Service Request */
#define SRC_ID_QSPI2RX			113	/* QSPI 2 Receive Service Request */
#define SRC_ID_QSPI2ERR			114	/* QSPI 2 Error Service Request */
#define SRC_ID_QSPI2PT			115	/* QSPI 2 Phase Transition Service Request */
#define SRC_ID_QSPI2HC			116	/* QSPI 2 Half Cycle Service Request */
#define SRC_ID_QSPI2U			117	/* QSPI 2 User Defined Service Request */
#define SRC_ID_QSPI3TX			118	/* QSPI 3 Transmit Service Request */
#define SRC_ID_QSPI3RX			119	/* QSPI 3 Receive Service Request */
#define SRC_ID_QSPI3ERR			120	/* QSPI 3 Error Service Request */
#define SRC_ID_QSPI3PT			121	/* QSPI 3 Phase Transition Service Request */
#define SRC_ID_QSPI3HC			122	/* QSPI 3 Half Cycle Service Request */
#define SRC_ID_QSPI3U			123	/* QSPI 3 User Defined Service Request */

#define SRC_ID_HSCT				164	/* HSCT Service Request */

#define SRC_ID_HSSLCOK0			168	/* HSSL Channel 0 OK Service Request */
#define SRC_ID_HSSLRDI0			169	/* HSSL Channel 0 Read Data Service Request */
#define SRC_ID_HSSLERR0			170	/* HSSL Channel 0 Error Service Request */
#define SRC_ID_HSSLTRG0			171	/* HSSL Channel 0 Trigger Service Request */
#define SRC_ID_HSSLCOK1			172	/* HSSL Channel 1 OK Service Request */
#define SRC_ID_HSSLRDI1			173	/* HSSL Channel 1 Read Data Service Request */
#define SRC_ID_HSSLERR1			174	/* HSSL Channel 1 Error Service Request */
#define SRC_ID_HSSLTRG1			175	/* HSSL Channel 1 Trigger Service Request */
#define SRC_ID_HSSLCOK2			176	/* HSSL Channel 2 OK Service Request */
#define SRC_ID_HSSLRDI2			177	/* HSSL Channel 2 Read Data Service Request */
#define SRC_ID_HSSLERR2			178	/* HSSL Channel 2 Error Service Request */
#define SRC_ID_HSSLTRG2			179	/* HSSL Channel 2 Trigger Service Request */
#define SRC_ID_HSSLCOK3			180	/* HSSL Channel 3 OK Service Request */
#define SRC_ID_HSSLRDI3			181	/* HSSL Channel 3 Read Data Service Request */
#define SRC_ID_HSSLERR3			182	/* HSSL Channel 3 Error Service Request */
#define SRC_ID_HSSLTRG3			183	/* HSSL Channel 3 Trigger Service Request */
#define SRC_ID_HSSLEXI			184	/* HSSL Exception Service Request */

#define SRC_ID_I2C0BREQ			192	/* I2C 0 Burst Data Transfer Request */
#define SRC_ID_I2C0LBREQ		193	/* I2C 0 Last Burst Data Transfer Request */
#define SRC_ID_I2C0SREQ			194	/* I2C 0 Single Data Transfer Request */
#define SRC_ID_I2C0LSREQ		195	/* I2C 0 Last Single Data Transfer Request */
#define SRC_ID_I2C0ERR			196	/* I2C 0 Error Service Request */
#define SRC_ID_I2C0P			197	/* I2C 0 Kernel Service Request */

#define SRC_ID_SENT0			212	/* SENT TRIG0 Service Request */
#define SRC_ID_SENT1			213	/* SENT TRIG1 Service Request */
#define SRC_ID_SENT2			214	/* SENT TRIG2 Service Request */
#define SRC_ID_SENT3			215	/* SENT TRIG3 Service Request */
#define SRC_ID_SENT4			216	/* SENT TRIG4 Service Request */
#define SRC_ID_SENT5			217	/* SENT TRIG5 Service Request */
#define SRC_ID_SENT6			218	/* SENT TRIG6 Service Request */
#define SRC_ID_SENT7			219	/* SENT TRIG7 Service Request */
#define SRC_ID_SENT8			220	/* SENT TRIG8 Service Request */
#define SRC_ID_SENT9			221	/* SENT TRIG9 Service Request */

#define SRC_ID_MSC0SR0			248	/* MSC 0 Service Request 0 */
#define SRC_ID_MSC0SR1			249	/* MSC 0 Service Request 1 */
#define SRC_ID_MSC0SR2			250	/* MSC 0 Service Request 2 */
#define SRC_ID_MSC0SR3			251	/* MSC 0 Service Request 3 */
#define SRC_ID_MSC0SR4			252	/* MSC 0 Service Request 4 */
#define SRC_ID_MSC1SR0			253	/* MSC 1 Service Request 0 */
#define SRC_ID_MSC1SR1			254	/* MSC 1 Service Request 1 */
#define SRC_ID_MSC1SR2			255	/* MSC 1 Service Request 2 */
#define SRC_ID_MSC1SR3			256	/* MSC 1 Service Request 3 */
#define SRC_ID_MSC1SR4			257	/* MSC 1 Service Request 4 */

#define SRC_ID_CCU60SR0			264	/* CCU6 0 Service Request 0 */
#define SRC_ID_CCU60SR1			265	/* CCU6 0 Service Request 1 */
#define SRC_ID_CCU60SR2			266	/* CCU6 0 Service Request 2 */
#define SRC_ID_CCU60SR3			267	/* CCU6 0 Service Request 3 */
#define SRC_ID_CCU61SR0			268	/* CCU6 1 Service Request 0 */
#define SRC_ID_CCU61SR1			269	/* CCU6 1 Service Request 1 */
#define SRC_ID_CCU61SR2			270	/* CCU6 1 Service Request 2 */
#define SRC_ID_CCU61SR3			271	/* CCU6 1 Service Request 3 */

#define SRC_ID_GPT120CIRQ		280	/* GPT120 CAPREL Service Request */
#define SRC_ID_GPT120T2			281	/* GPT120 T2 Overflow/Underflow Service Request */
#define SRC_ID_GPT120T3			282	/* GPT120 T3 Overflow/Underflow Service Request */
#define SRC_ID_GPT120T4			283	/* GPT120 T4 Overflow/Underflow Service Request */
#define SRC_ID_GPT120T5			284	/* GPT120 T5 Overflow/Underflow Service Request */
#define SRC_ID_GPT120T6			285	/* GPT120 T6 Overflow/Underflow Service Request */

#define SRC_ID_STM0SR0			292	/* System Timer 0 Service Request 0 */
#define SRC_ID_STM0SR1			293	/* System Timer 0 Service Request 1 */
#define SRC_ID_STM1SR0			294	/* System Timer 1 Service Request 0 */
#define SRC_ID_STM1SR1			295	/* System Timer 1 Service Request 1 */
#define SRC_ID_STM2SR0			296	/* System Timer 2 Service Request 0 */
#define SRC_ID_STM2SR1			297	/* System Timer 2 Service Request 1 */

#define SRC_ID_FCE				300	/* FCE Error Service Request */

#define SRC_ID_DMAERR			316	/* DMA Error Service Request */

#define SRC_ID_DMACH0			320	/* DMA Channel  0 Service Request */
#define SRC_ID_DMACH1			321	/* DMA Channel  1 Service Request */
#define SRC_ID_DMACH2			322	/* DMA Channel  2 Service Request */
#define SRC_ID_DMACH3			323	/* DMA Channel  3 Service Request */
#define SRC_ID_DMACH4			324	/* DMA Channel  4 Service Request */
#define SRC_ID_DMACH5			325	/* DMA Channel  5 Service Request */
#define SRC_ID_DMACH6			326	/* DMA Channel  6 Service Request */
#define SRC_ID_DMACH7			327	/* DMA Channel  7 Service Request */
#define SRC_ID_DMACH8			328	/* DMA Channel  8 Service Request */
#define SRC_ID_DMACH9			329	/* DMA Channel  9 Service Request */
#define SRC_ID_DMACH10			330	/* DMA Channel 10 Service Request */
#define SRC_ID_DMACH11			331	/* DMA Channel 11 Service Request */
#define SRC_ID_DMACH12			332	/* DMA Channel 12 Service Request */
#define SRC_ID_DMACH13			333	/* DMA Channel 13 Service Request */
#define SRC_ID_DMACH14			334	/* DMA Channel 14 Service Request */
#define SRC_ID_DMACH15			335	/* DMA Channel 15 Service Request */
#define SRC_ID_DMACH16			336	/* DMA Channel 16 Service Request */
#define SRC_ID_DMACH17			337	/* DMA Channel 17 Service Request */
#define SRC_ID_DMACH18			338	/* DMA Channel 18 Service Request */
#define SRC_ID_DMACH19			339	/* DMA Channel 19 Service Request */
#define SRC_ID_DMACH20			340	/* DMA Channel 20 Service Request */
#define SRC_ID_DMACH21			341	/* DMA Channel 21 Service Request */
#define SRC_ID_DMACH22			342	/* DMA Channel 22 Service Request */
#define SRC_ID_DMACH23			343	/* DMA Channel 23 Service Request */
#define SRC_ID_DMACH24			344	/* DMA Channel 24 Service Request */
#define SRC_ID_DMACH25			345	/* DMA Channel 25 Service Request */
#define SRC_ID_DMACH26			346	/* DMA Channel 26 Service Request */
#define SRC_ID_DMACH27			347	/* DMA Channel 27 Service Request */
#define SRC_ID_DMACH28			348	/* DMA Channel 28 Service Request */
#define SRC_ID_DMACH29			349	/* DMA Channel 29 Service Request */
#define SRC_ID_DMACH30			350	/* DMA Channel 30 Service Request */
#define SRC_ID_DMACH31			351	/* DMA Channel 31 Service Request */
#define SRC_ID_DMACH32			352	/* DMA Channel 32 Service Request */
#define SRC_ID_DMACH33			353	/* DMA Channel 33 Service Request */
#define SRC_ID_DMACH34			354	/* DMA Channel 34 Service Request */
#define SRC_ID_DMACH35			355	/* DMA Channel 35 Service Request */
#define SRC_ID_DMACH36			356	/* DMA Channel 36 Service Request */
#define SRC_ID_DMACH37			357	/* DMA Channel 37 Service Request */
#define SRC_ID_DMACH38			358	/* DMA Channel 38 Service Request */
#define SRC_ID_DMACH39			359	/* DMA Channel 39 Service Request */
#define SRC_ID_DMACH40			360	/* DMA Channel 40 Service Request */
#define SRC_ID_DMACH41			361	/* DMA Channel 41 Service Request */
#define SRC_ID_DMACH42			362	/* DMA Channel 42 Service Request */
#define SRC_ID_DMACH43			363	/* DMA Channel 43 Service Request */
#define SRC_ID_DMACH44			364	/* DMA Channel 44 Service Request */
#define SRC_ID_DMACH45			365	/* DMA Channel 45 Service Request */
#define SRC_ID_DMACH46			366	/* DMA Channel 46 Service Request */
#define SRC_ID_DMACH47			367	/* DMA Channel 47 Service Request */
#define SRC_ID_DMACH48			368	/* DMA Channel 48 Service Request */
#define SRC_ID_DMACH49			369	/* DMA Channel 49 Service Request */
#define SRC_ID_DMACH50			370	/* DMA Channel 50 Service Request */
#define SRC_ID_DMACH51			371	/* DMA Channel 51 Service Request */
#define SRC_ID_DMACH52			372	/* DMA Channel 52 Service Request */
#define SRC_ID_DMACH53			373	/* DMA Channel 53 Service Request */
#define SRC_ID_DMACH54			374	/* DMA Channel 54 Service Request */
#define SRC_ID_DMACH55			375	/* DMA Channel 55 Service Request */
#define SRC_ID_DMACH56			376	/* DMA Channel 56 Service Request */
#define SRC_ID_DMACH57			377	/* DMA Channel 57 Service Request */
#define SRC_ID_DMACH58			378	/* DMA Channel 58 Service Request */
#define SRC_ID_DMACH59			379	/* DMA Channel 59 Service Request */
#define SRC_ID_DMACH60			380	/* DMA Channel 60 Service Request */
#define SRC_ID_DMACH61			381	/* DMA Channel 61 Service Request */
#define SRC_ID_DMACH62			382	/* DMA Channel 62 Service Request */
#define SRC_ID_DMACH63			383	/* DMA Channel 63 Service Request */

#define SRC_ID_ETH				572	/* Ethernet Service Request */

#define SRC_ID_CANINT0			576	/* MultiCAN+ Service Request 0 */
#define SRC_ID_CANINT1			577	/* MultiCAN+ Service Request 1 */
#define SRC_ID_CANINT2			578	/* MultiCAN+ Service Request 2 */
#define SRC_ID_CANINT3			579	/* MultiCAN+ Service Request 3 */
#define SRC_ID_CANINT4			580	/* MultiCAN+ Service Request 4 */
#define SRC_ID_CANINT5			581	/* MultiCAN+ Service Request 5 */
#define SRC_ID_CANINT6			582	/* MultiCAN+ Service Request 6 */
#define SRC_ID_CANINT7			583	/* MultiCAN+ Service Request 7 */
#define SRC_ID_CANINT8			584	/* MultiCAN+ Service Request 8 */
#define SRC_ID_CANINT9			585	/* MultiCAN+ Service Request 9 */
#define SRC_ID_CANINT10			586	/* MultiCAN+ Service Request 10 */
#define SRC_ID_CANINT11			587	/* MultiCAN+ Service Request 11 */
#define SRC_ID_CANINT12			588	/* MultiCAN+ Service Request 12 */
#define SRC_ID_CANINT13			589	/* MultiCAN+ Service Request 13 */
#define SRC_ID_CANINT14			590	/* MultiCAN+ Service Request 14 */
#define SRC_ID_CANINT15			591	/* MultiCAN+ Service Request 15 */

#define SRC_ID_VADCG0SR0		608	/* VADC Group 0 Service Request 0 */
#define SRC_ID_VADCG0SR1		609	/* VADC Group 0 Service Request 1 */
#define SRC_ID_VADCG0SR2		610	/* VADC Group 0 Service Request 2 */
#define SRC_ID_VADCG0SR3		611	/* VADC Group 0 Service Request 3 */
#define SRC_ID_VADCG1SR0		612	/* VADC Group 1 Service Request 0 */
#define SRC_ID_VADCG1SR1		613	/* VADC Group 1 Service Request 1 */
#define SRC_ID_VADCG1SR2		614	/* VADC Group 1 Service Request 2 */
#define SRC_ID_VADCG1SR3		615	/* VADC Group 1 Service Request 3 */
#define SRC_ID_VADCG2SR0		616	/* VADC Group 2 Service Request 0 */
#define SRC_ID_VADCG2SR1		617	/* VADC Group 2 Service Request 1 */
#define SRC_ID_VADCG2SR2		618	/* VADC Group 2 Service Request 2 */
#define SRC_ID_VADCG2SR3		619	/* VADC Group 2 Service Request 3 */
#define SRC_ID_VADCG3SR0		620	/* VADC Group 3 Service Request 0 */
#define SRC_ID_VADCG3SR1		621	/* VADC Group 3 Service Request 1 */
#define SRC_ID_VADCG3SR2		622	/* VADC Group 3 Service Request 2 */
#define SRC_ID_VADCG3SR3		623	/* VADC Group 3 Service Request 3 */
#define SRC_ID_VADCG4SR0		624	/* VADC Group 4 Service Request 0 */
#define SRC_ID_VADCG4SR1		625	/* VADC Group 4 Service Request 1 */
#define SRC_ID_VADCG4SR2		626	/* VADC Group 4 Service Request 2 */
#define SRC_ID_VADCG4SR3		627	/* VADC Group 4 Service Request 3 */
#define SRC_ID_VADCG5SR0		628	/* VADC Group 5 Service Request 0 */
#define SRC_ID_VADCG5SR1		629	/* VADC Group 5 Service Request 1 */
#define SRC_ID_VADCG5SR2		630	/* VADC Group 5 Service Request 2 */
#define SRC_ID_VADCG5SR3		631	/* VADC Group 5 Service Request 3 */
#define SRC_ID_VADCG6SR0		632	/* VADC Group 6 Service Request 0 */
#define SRC_ID_VADCG6SR1		633	/* VADC Group 6 Service Request 1 */
#define SRC_ID_VADCG6SR2		634	/* VADC Group 6 Service Request 2 */
#define SRC_ID_VADCG6SR3		635	/* VADC Group 6 Service Request 3 */
#define SRC_ID_VADCG7SR0		636	/* VADC Group 7 Service Request 0 */
#define SRC_ID_VADCG7SR1		637	/* VADC Group 7 Service Request 1 */
#define SRC_ID_VADCG7SR2		638	/* VADC Group 7 Service Request 2 */
#define SRC_ID_VADCG7SR3		639	/* VADC Group 7 Service Request 3 */

#define SRC_ID_VADCCG0SR0		680	/* VADC Common Group 0 Service Request 0 */
#define SRC_ID_VADCCG0SR1		681	/* VADC Common Group 0 Service Request 1 */
#define SRC_ID_VADCCG0SR2		682	/* VADC Common Group 0 Service Request 2 */
#define SRC_ID_VADCCG0SR3		683	/* VADC Common Group 0 Service Request 3 */
#define SRC_ID_VADCCG1SR0		684	/* VADC Common Group 1 Service Request 0 */
#define SRC_ID_VADCCG1SR1		685	/* VADC Common Group 1 Service Request 1 */
#define SRC_ID_VADCCG1SR2		686	/* VADC Common Group 1 Service Request 2 */
#define SRC_ID_VADCCG1SR3		687	/* VADC Common Group 1 Service Request 3 */

#define SRC_ID_DSADCSRM0		724	/* DSADC SRM0 Service Request */
#define SRC_ID_DSADCSRA0		725	/* DSADC SRA0 Service Request */
#define SRC_ID_DSADCSRM1		726	/* DSADC SRM1 Service Request */
#define SRC_ID_DSADCSRA1		727	/* DSADC SRA1 Service Request */
#define SRC_ID_DSADCSRM2		728	/* DSADC SRM2 Service Request */
#define SRC_ID_DSADCSRA2		729	/* DSADC SRA2 Service Request */
#define SRC_ID_DSADCSRM3		730	/* DSADC SRM3 Service Request */
#define SRC_ID_DSADCSRA3		731	/* DSADC SRA3 Service Request */
#define SRC_ID_DSADCSRM4		732	/* DSADC SRM4 Service Request */
#define SRC_ID_DSADCSRA4		733	/* DSADC SRA4 Service Request */
#define SRC_ID_DSADCSRM5		734	/* DSADC SRM5 Service Request */
#define SRC_ID_DSADCSRA5		735	/* DSADC SRA5 Service Request */

#define SRC_ID_ERAYINT0			760	/* E-RAY Service Request 0 */
#define SRC_ID_ERAYINT1			761	/* E-RAY Service Request 1 */
#define SRC_ID_ERAYTINT0		762	/* E-RAY Timer Interrupt 0 Service Request */
#define SRC_ID_ERAYTINT1		763	/* E-RAY Timer Interrupt 1 Service Request */
#define SRC_ID_ERAYNDAT0		764	/* E-RAY New Data 0 Service Request */
#define SRC_ID_ERAYNDAT1		765	/* E-RAY New Data 1 Service Request */
#define SRC_ID_ERAYMBSC0		766	/* E-RAY Message Buffer Status Changed 0 Service Request */
#define SRC_ID_ERAYMBSC1		767	/* E-RAY Message Buffer Status Changed 1 Service Request */
#define SRC_ID_ERAYOBUSY		768	/* E-RAY Output Buffer Busy Service Request */
#define SRC_ID_ERAYIBUSY		769	/* E-RAY Input Buffer Busy Service Request */

#define SRC_ID_PMU00			780	/* PMU 0 Service Request 0 */
#define SRC_ID_PMU01			781	/* PMU 0 Service Request 1 */

#define SRC_ID_HSM0				816	/* HSM Service Request 0 */
#define SRC_ID_HSM1				817	/* HSM Service Request 1 */

#define SRC_ID_SCUDTS			820	/* SCU DTS Busy Service Request */
#define SRC_ID_SCUERU0			821	/* SCU ERU Service Request 0 */
#define SRC_ID_SCUERU1			822	/* SCU ERU Service Request 1 */
#define SRC_ID_SCUERU2			823	/* SCU ERU Service Request 2 */
#define SRC_ID_SCUERU3			824	/* SCU ERU Service Request 3 */

#define SRC_ID_SMU0				836	/* SMU Service Request 0 */
#define SRC_ID_SMU1				837	/* SMU Service Request 1 */
#define SRC_ID_SMU2				838	/* SMU Service Request 2 */

#define SRC_ID_PSI50			844	/* PSI5 Service Request 0 */
#define SRC_ID_PSI51			845	/* PSI5 Service Request 1 */
#define SRC_ID_PSI52			846	/* PSI5 Service Request 2 */
#define SRC_ID_PSI53			847	/* PSI5 Service Request 3 */
#define SRC_ID_PSI54			848	/* PSI5 Service Request 4 */
#define SRC_ID_PSI55			849	/* PSI5 Service Request 5 */
#define SRC_ID_PSI56			850	/* PSI5 Service Request 6 */
#define SRC_ID_PSI57			851	/* PSI5 Service Request 7 */

#define SRC_ID_DAMLI0			860	/* DAM Limit 0 Service Request */
#define SRC_ID_DAMRI0			861	/* DAM Ready 0 Service Request */
#define SRC_ID_DAMLI1			862	/* DAM Limit 1 Service Request */
#define SRC_ID_DAMRI1			863	/* DAM Ready 1 Service Request */
#define SRC_ID_DAMDR			864	/* DAM DMA Ready Service Request */
#define SRC_ID_DAMERR			865	/* DAM Error Service Request */

#define SRC_ID_CIFMI			872	/* CIF MI Service Request */
#define SRC_ID_CIFMIEP			873	/* CIF MI EP Service Request */
#define SRC_ID_CIFISP			874	/* CIF ISP Service Request */
#define SRC_ID_CIFMJPEG			875	/* CIF MJPEG Service Request */

#define SRC_ID_LMU				888	/* LMU Error Service Request */

#define SRC_ID_PSI5S0			892	/* PSI5-S Service Request 0 */
#define SRC_ID_PSI5S1			893	/* PSI5-S Service Request 1 */
#define SRC_ID_PSI5S2			894	/* PSI5-S Service Request 2 */
#define SRC_ID_PSI5S3			895	/* PSI5-S Service Request 3 */
#define SRC_ID_PSI5S4			896	/* PSI5-S Service Request 4 */
#define SRC_ID_PSI5S5			897	/* PSI5-S Service Request 5 */
#define SRC_ID_PSI5S6			898	/* PSI5-S Service Request 6 */
#define SRC_ID_PSI5S7			899	/* PSI5-S Service Request 7 */

#define SRC_ID_GPSR00			1024	/* General Purpose Service Request 0 0 */
#define SRC_ID_GPSR01			1025	/* General Purpose Service Request 0 1 */
#define SRC_ID_GPSR02			1026	/* General Purpose Service Request 0 2 */
#define SRC_ID_GPSR03			1027	/* General Purpose Service Request 0 3 */

#define SRC_ID_GPSR10			1032	/* General Purpose Service Request 1 0 */
#define SRC_ID_GPSR11			1033	/* General Purpose Service Request 1 1 */
#define SRC_ID_GPSR12			1034	/* General Purpose Service Request 1 2 */
#define SRC_ID_GPSR13			1035	/* General Purpose Service Request 1 3 */

#define SRC_ID_GPSR20			1040	/* General Purpose Service Request 2 0 */
#define SRC_ID_GPSR21			1041	/* General Purpose Service Request 2 1 */
#define SRC_ID_GPSR22			1042	/* General Purpose Service Request 2 2 */
#define SRC_ID_GPSR23			1043	/* General Purpose Service Request 2 3 */

#define SRC_ID_GTMAEIIRQ		1408	/* GTM AEI Shared Service Request */
#define SRC_ID_GTMARUIRQ0		1409	/* GTM ARU Shared Service Request 0 */
#define SRC_ID_GTMARUIRQ1		1410	/* GTM ARU Shared Service Request 1 */
#define SRC_ID_GTMARUIRQ2		1411	/* GTM ARU Shared Service Request 2 */

#define SRC_ID_GTMBRCIRQ		1413	/* GTM BRC Shared Service Request */
#define SRC_ID_GTMCMPIRQ		1414	/* GTM CMP Shared Service Request */
#define SRC_ID_GTMSPE0IRQ		1415	/* GTM SPE0 Shared Service Request */
#define SRC_ID_GTMSPE1IRQ		1416	/* GTM SPE1 Shared Service Request */

#define SRC_ID_GTMPSM00			1419	/* GTM PSM0 Shared Service Request 0 */
#define SRC_ID_GTMPSM01			1420	/* GTM PSM0 Shared Service Request 1 */
#define SRC_ID_GTMPSM02			1421	/* GTM PSM0 Shared Service Request 2 */
#define SRC_ID_GTMPSM03			1422	/* GTM PSM0 Shared Service Request 3 */
#define SRC_ID_GTMPSM04			1423	/* GTM PSM0 Shared Service Request 4 */
#define SRC_ID_GTMPSM05			1424	/* GTM PSM0 Shared Service Request 5 */
#define SRC_ID_GTMPSM06			1425	/* GTM PSM0 Shared Service Request 6 */
#define SRC_ID_GTMPSM07			1426	/* GTM PSM0 Shared Service Request 7 */

#define SRC_ID_GTMDPLL0			1449	/* GTM DPLL Service Request 0 */
#define SRC_ID_GTMDPLL1			1450	/* GTM DPLL Service Request 1 */
#define SRC_ID_GTMDPLL2			1451	/* GTM DPLL Service Request 2 */
#define SRC_ID_GTMDPLL3			1452	/* GTM DPLL Service Request 3 */
#define SRC_ID_GTMDPLL4			1453	/* GTM DPLL Service Request 4 */
#define SRC_ID_GTMDPLL5			1454	/* GTM DPLL Service Request 5 */
#define SRC_ID_GTMDPLL6			1455	/* GTM DPLL Service Request 6 */
#define SRC_ID_GTMDPLL7			1456	/* GTM DPLL Service Request 7 */
#define SRC_ID_GTMDPLL8			1457	/* GTM DPLL Service Request 8 */
#define SRC_ID_GTMDPLL9			1458	/* GTM DPLL Service Request 9 */
#define SRC_ID_GTMDPLL10		1459	/* GTM DPLL Service Request 10 */
#define SRC_ID_GTMDPLL11		1460	/* GTM DPLL Service Request 11 */
#define SRC_ID_GTMDPLL12		1461	/* GTM DPLL Service Request 12 */
#define SRC_ID_GTMDPLL13		1462	/* GTM DPLL Service Request 13 */
#define SRC_ID_GTMDPLL14		1463	/* GTM DPLL Service Request 14 */
#define SRC_ID_GTMDPLL15		1464	/* GTM DPLL Service Request 15 */
#define SRC_ID_GTMDPLL16		1465	/* GTM DPLL Service Request 16 */
#define SRC_ID_GTMDPLL17		1466	/* GTM DPLL Service Request 17 */
#define SRC_ID_GTMDPLL18		1467	/* GTM DPLL Service Request 18 */
#define SRC_ID_GTMDPLL19		1468	/* GTM DPLL Service Request 19 */
#define SRC_ID_GTMDPLL20		1469	/* GTM DPLL Service Request 20 */
#define SRC_ID_GTMDPLL21		1470	/* GTM DPLL Service Request 21 */
#define SRC_ID_GTMDPLL22		1471	/* GTM DPLL Service Request 22 */
#define SRC_ID_GTMDPLL23		1472	/* GTM DPLL Service Request 23 */
#define SRC_ID_GTMDPLL24		1473	/* GTM DPLL Service Request 24 */
#define SRC_ID_GTMDPLL25		1474	/* GTM DPLL Service Request 25 */
#define SRC_ID_GTMDPLL26		1475	/* GTM DPLL Service Request 26 */

#define SRC_ID_GTMERR			1500	/* GTM Error Service Request */

#define SRC_ID_GTMTIM00			1504	/* GTM TIM0 Shared Service Request 0 */
#define SRC_ID_GTMTIM01			1505	/* GTM TIM0 Shared Service Request 1 */
#define SRC_ID_GTMTIM02			1506	/* GTM TIM0 Shared Service Request 2 */
#define SRC_ID_GTMTIM03			1507	/* GTM TIM0 Shared Service Request 3 */
#define SRC_ID_GTMTIM04			1508	/* GTM TIM0 Shared Service Request 4 */
#define SRC_ID_GTMTIM05			1509	/* GTM TIM0 Shared Service Request 5 */
#define SRC_ID_GTMTIM06			1510	/* GTM TIM0 Shared Service Request 6 */
#define SRC_ID_GTMTIM07			1511	/* GTM TIM0 Shared Service Request 7 */
#define SRC_ID_GTMTIM10			1512	/* GTM TIM1 Shared Service Request 0 */
#define SRC_ID_GTMTIM11			1513	/* GTM TIM1 Shared Service Request 1 */
#define SRC_ID_GTMTIM12			1514	/* GTM TIM1 Shared Service Request 2 */
#define SRC_ID_GTMTIM13			1515	/* GTM TIM1 Shared Service Request 3 */
#define SRC_ID_GTMTIM14			1516	/* GTM TIM1 Shared Service Request 4 */
#define SRC_ID_GTMTIM15			1517	/* GTM TIM1 Shared Service Request 5 */
#define SRC_ID_GTMTIM16			1518	/* GTM TIM1 Shared Service Request 6 */
#define SRC_ID_GTMTIM17			1519	/* GTM TIM1 Shared Service Request 7 */
#define SRC_ID_GTMTIM20			1520	/* GTM TIM2 Shared Service Request 0 */
#define SRC_ID_GTMTIM21			1521	/* GTM TIM2 Shared Service Request 1 */
#define SRC_ID_GTMTIM22			1522	/* GTM TIM2 Shared Service Request 2 */
#define SRC_ID_GTMTIM23			1523	/* GTM TIM2 Shared Service Request 3 */
#define SRC_ID_GTMTIM24			1524	/* GTM TIM2 Shared Service Request 4 */
#define SRC_ID_GTMTIM25			1525	/* GTM TIM2 Shared Service Request 5 */
#define SRC_ID_GTMTIM26			1526	/* GTM TIM2 Shared Service Request 6 */
#define SRC_ID_GTMTIM27			1527	/* GTM TIM2 Shared Service Request 7 */
#define SRC_ID_GTMTIM30			1528	/* GTM TIM3 Shared Service Request 0 */
#define SRC_ID_GTMTIM31			1529	/* GTM TIM3 Shared Service Request 1 */
#define SRC_ID_GTMTIM32			1530	/* GTM TIM3 Shared Service Request 2 */
#define SRC_ID_GTMTIM33			1531	/* GTM TIM3 Shared Service Request 3 */
#define SRC_ID_GTMTIM34			1532	/* GTM TIM3 Shared Service Request 4 */
#define SRC_ID_GTMTIM35			1533	/* GTM TIM3 Shared Service Request 5 */
#define SRC_ID_GTMTIM36			1534	/* GTM TIM3 Shared Service Request 6 */
#define SRC_ID_GTMTIM37			1535	/* GTM TIM3 Shared Service Request 7 */

#define SRC_ID_GTMMSC00			1632	/* GTM MCS0 Shared Service Request 0 */
#define SRC_ID_GTMMSC01			1633	/* GTM MCS0 Shared Service Request 1 */
#define SRC_ID_GTMMSC02			1634	/* GTM MCS0 Shared Service Request 2 */
#define SRC_ID_GTMMSC03			1635	/* GTM MCS0 Shared Service Request 3 */
#define SRC_ID_GTMMSC04			1636	/* GTM MCS0 Shared Service Request 4 */
#define SRC_ID_GTMMSC05			1637	/* GTM MCS0 Shared Service Request 5 */
#define SRC_ID_GTMMSC06			1638	/* GTM MCS0 Shared Service Request 6 */
#define SRC_ID_GTMMSC07			1639	/* GTM MCS0 Shared Service Request 7 */
#define SRC_ID_GTMMSC10			1640	/* GTM MCS1 Shared Service Request 0 */
#define SRC_ID_GTMMSC11			1641	/* GTM MCS1 Shared Service Request 1 */
#define SRC_ID_GTMMSC12			1642	/* GTM MCS1 Shared Service Request 2 */
#define SRC_ID_GTMMSC13			1643	/* GTM MCS1 Shared Service Request 3 */
#define SRC_ID_GTMMSC14			1644	/* GTM MCS1 Shared Service Request 4 */
#define SRC_ID_GTMMSC15			1645	/* GTM MCS1 Shared Service Request 5 */
#define SRC_ID_GTMMSC16			1646	/* GTM MCS1 Shared Service Request 6 */
#define SRC_ID_GTMMSC17			1647	/* GTM MCS1 Shared Service Request 7 */
#define SRC_ID_GTMMSC20			1648	/* GTM MCS2 Shared Service Request 0 */
#define SRC_ID_GTMMSC21			1649	/* GTM MCS2 Shared Service Request 1 */
#define SRC_ID_GTMMSC22			1650	/* GTM MCS2 Shared Service Request 2 */
#define SRC_ID_GTMMSC23			1651	/* GTM MCS2 Shared Service Request 3 */
#define SRC_ID_GTMMSC24			1652	/* GTM MCS2 Shared Service Request 4 */
#define SRC_ID_GTMMSC25			1653	/* GTM MCS2 Shared Service Request 5 */
#define SRC_ID_GTMMSC26			1654	/* GTM MCS2 Shared Service Request 6 */
#define SRC_ID_GTMMSC27			1655	/* GTM MCS2 Shared Service Request 7 */
#define SRC_ID_GTMMSC30			1656	/* GTM MCS3 Shared Service Request 0 */
#define SRC_ID_GTMMSC31			1657	/* GTM MCS3 Shared Service Request 1 */
#define SRC_ID_GTMMSC32			1658	/* GTM MCS3 Shared Service Request 2 */
#define SRC_ID_GTMMSC33			1659	/* GTM MCS3 Shared Service Request 3 */
#define SRC_ID_GTMMSC34			1660	/* GTM MCS3 Shared Service Request 4 */
#define SRC_ID_GTMMSC35			1661	/* GTM MCS3 Shared Service Request 5 */
#define SRC_ID_GTMMSC36			1662	/* GTM MCS3 Shared Service Request 6 */
#define SRC_ID_GTMMSC37			1663	/* GTM MCS3 Shared Service Request 7 */

#define SRC_ID_GTMTOM00			1760	/* GTM TOM0 Shared Service Request 0 */
#define SRC_ID_GTMTOM01			1761	/* GTM TOM0 Shared Service Request 1 */
#define SRC_ID_GTMTOM02			1762	/* GTM TOM0 Shared Service Request 2 */
#define SRC_ID_GTMTOM03			1763	/* GTM TOM0 Shared Service Request 3 */
#define SRC_ID_GTMTOM04			1764	/* GTM TOM0 Shared Service Request 4 */
#define SRC_ID_GTMTOM05			1765	/* GTM TOM0 Shared Service Request 5 */
#define SRC_ID_GTMTOM06			1766	/* GTM TOM0 Shared Service Request 6 */
#define SRC_ID_GTMTOM07			1767	/* GTM TOM0 Shared Service Request 7 */
#define SRC_ID_GTMTOM10			1768	/* GTM TOM1 Shared Service Request 0 */
#define SRC_ID_GTMTOM11			1769	/* GTM TOM1 Shared Service Request 1 */
#define SRC_ID_GTMTOM12			1770	/* GTM TOM1 Shared Service Request 2 */
#define SRC_ID_GTMTOM13			1771	/* GTM TOM1 Shared Service Request 3 */
#define SRC_ID_GTMTOM14			1772	/* GTM TOM1 Shared Service Request 4 */
#define SRC_ID_GTMTOM15			1773	/* GTM TOM1 Shared Service Request 5 */
#define SRC_ID_GTMTOM16			1774	/* GTM TOM1 Shared Service Request 6 */
#define SRC_ID_GTMTOM17			1775	/* GTM TOM1 Shared Service Request 7 */
#define SRC_ID_GTMTOM20			1776	/* GTM TOM2 Shared Service Request 0 */
#define SRC_ID_GTMTOM21			1777	/* GTM TOM2 Shared Service Request 1 */
#define SRC_ID_GTMTOM22			1778	/* GTM TOM2 Shared Service Request 2 */
#define SRC_ID_GTMTOM23			1779	/* GTM TOM2 Shared Service Request 3 */
#define SRC_ID_GTMTOM24			1780	/* GTM TOM2 Shared Service Request 4 */
#define SRC_ID_GTMTOM25			1781	/* GTM TOM2 Shared Service Request 5 */
#define SRC_ID_GTMTOM26			1782	/* GTM TOM2 Shared Service Request 6 */
#define SRC_ID_GTMTOM27			1783	/* GTM TOM2 Shared Service Request 7 */

#define SRC_ID_GTMATOM00		1888	/* GTM ATOM0 Shared Service Request 0 */
#define SRC_ID_GTMATOM01		1889	/* GTM ATOM0 Shared Service Request 1 */
#define SRC_ID_GTMATOM02		1890	/* GTM ATOM0 Shared Service Request 2 */
#define SRC_ID_GTMATOM03		1891	/* GTM ATOM0 Shared Service Request 3 */
#define SRC_ID_GTMATOM10		1892	/* GTM ATOM1 Shared Service Request 0 */
#define SRC_ID_GTMATOM11		1893	/* GTM ATOM1 Shared Service Request 1 */
#define SRC_ID_GTMATOM12		1894	/* GTM ATOM1 Shared Service Request 2 */
#define SRC_ID_GTMATOM13		1895	/* GTM ATOM1 Shared Service Request 3 */
#define SRC_ID_GTMATOM20		1896	/* GTM ATOM2 Shared Service Request 0 */
#define SRC_ID_GTMATOM21		1897	/* GTM ATOM2 Shared Service Request 1 */
#define SRC_ID_GTMATOM22		1898	/* GTM ATOM2 Shared Service Request 2 */
#define SRC_ID_GTMATOM23		1899	/* GTM ATOM2 Shared Service Request 3 */
#define SRC_ID_GTMATOM30		1900	/* GTM ATOM3 Shared Service Request 0 */
#define SRC_ID_GTMATOM31		1901	/* GTM ATOM3 Shared Service Request 1 */
#define SRC_ID_GTMATOM32		1902	/* GTM ATOM3 Shared Service Request 2 */
#define SRC_ID_GTMATOM33		1903	/* GTM ATOM3 Shared Service Request 3 */
#define SRC_ID_GTMATOM40		1904	/* GTM ATOM4 Shared Service Request 0 */
#define SRC_ID_GTMATOM41		1905	/* GTM ATOM4 Shared Service Request 1 */
#define SRC_ID_GTMATOM42		1906	/* GTM ATOM4 Shared Service Request 2 */
#define SRC_ID_GTMATOM43		1907	/* GTM ATOM4 Shared Service Request 3 */

#define IRQ_ID_MAX_NUM			1908


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/* type of an Interrupt Service Routine (ISR) */
typedef void (*isrhnd_t)(int arg);


/*---------------------------------------------------------------------
	Function:	InterruptInit
	Purpose:	Initialisation of interrupt handling
	Arguments:	void
	Return:		void
---------------------------------------------------------------------*/
void InterruptInit(void);

/*---------------------------------------------------------------------
	Function:	InterruptInstall
	Purpose:	Install a service handler for an interrupt
	Arguments:	int irqNum       - number of interrupt
				isrhnd_t isrPrc - pointer to service routine
				int prio         - priority (1-255)
				int arg          - argument for service routine
	Return:		void
---------------------------------------------------------------------*/
void InterruptInstall(int irqNum, isrhnd_t isrPrc, int prio, int arg);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __INTERRUPTS_H__ */
