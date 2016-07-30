
/* $Id: put.c,v 1.14 2000/03/08 21:45:27 alex Exp $ */

/*******************************************************************
 * AUTHOR : Alexandre Fenyö              (Alexandre.Fenyo@lip6.fr) *
 * support for PCIDDC 1st Run with help of Franck Wasjburt         *
 *                                       (Franck.Wasjburt@lip6.fr) *
 * support for PCIDDC 2nd Run with help of Cyril Spasevski         *
 *                                       (Cyril.Spasevski@lip6.fr) *
 *******************************************************************/

/*

  Modèle de programmation de PUT :

  - PUT_MODEL_INTERRUPT_DRIVEN :

    on autorise pour l'émission :
      NONE    : page intermédiaire d'un message
      LMP|NOS : dernière page d'un message avec interrupt
      LMP     : dernière page d'un message sans interrupt
    Rq1: si on n'a jamais NOS, on devra supprimer le flag
         PUT_DONT_FLUSH_LPE, pour que la LPE puisse être vidée,
         ou appeler régulièrement une fonction qui la vide (par ex
         un fonction qui signale les entrées consommées : put_flush_lpe())
    Rq2: on sait qu'un message sans interrupt est parti
         si le message suivant (avec interrupt) est aussi parti
         (ou en utilisant la fonction dont on parle dans la rq 1)

    on interdit pour l'émission :
      NOS     : interrupt pour page intermédiaire


    on autorise pour la réception :
      NONE    : page intermédiaire d'un message ou
                dernière page d'un message sans interrupt
      LMI|NOR : dernière page d'un message avec interrupt
      LMI     : dernière page d'un message sans interrupt
    Rq1: les entrées de LMI sans interrupt sont néanmoins
         signalées au handler d'interrupt. De plus, put_flush_lmi()
	 force le signalement.

    on interdit pour la réception :
      NOR     : interrupt sans entrée de LMI associée



  - PUT_MODEL_POLLING :
    il n'y a aucune interruption.

    on autorise pour l'émission :
      NONE : page intermédiaire d'un message
      LMP  : dernière page d'un message
    Rq1: on est prévenu que la page est partie en consultant
         le registre LPE_CURRENT et en s'assurant que l'entrée
         qui suit a bien été consommée. IL Y A UNE AUTRE MÉTHODE :
         on supprime le masque d'interrupt global sur PAGE_TRANSMITTED,
         et on met NOS, ce qui a pour effet d'incrémenter le compteur
         de page transmise SANS générer d'interrupt -> on consulte
         et acquitte alors le compteur en question.
    Rq2: ca ne résoud pas le pb de flusher la LPE. Il faut la flusher
         par ex avec l'appel d'une fonction qui indiquerait les entrées
         consommées (put_flush_lpe()), ou en retirant PUT_DONT_FLUSH_LPE

    on autorise pour la réception :
      NONE : page intermédiaire
      LMI  : dernière page d'un message qui
             signale la réception dans la LMI
    Rq: on est prévenu que la page est reçu en consultant la LMI ou
        le dernier octet du tampon de réception. Une autre méthode :
	on appelle put_flush_lmi() qui force le signalement des
	entrées de LMI.


  D'autre part :
  - PUT_DONT_FLUSH_LPE :
    on ne vide pas la LPE au début de chaque put_add_entry().

  - PUT_OPTIMIZE :
    on optimize en supprimant des tests, afin de faire des benchs
    Rq: pour optimiser encore plus,
        on peut rajouter PUT_DONT_FLUSH_LPE
        et faire un configure --disable-stats

  SLR nécessite PUT_MODEL_INTERRUPT_DRIVEN
  (et PUT_DONT_FLUSH_LPE peut être positionné)



  PRINCIPE GLOBAL :

  DISTINGUER INTERRUPTION ET SIGNALISATION

  - Dans le modèle PUT_MODEL_INTERRUPT_DRIVEN, des interruptions
    sont générées si la signalisation est demandée (NOR, NOS)
  - Dans le modèle PUT_MODEL_POLLING, aucune interruption n'est générée
    grace au masque global des causes d'interruptions de PCIDDC.
    On va utiliser néanmoins NOS pour signaler les émissions. Toutes
    les réceptions avec LMI seront signalées.
  - Dans tous les cas : on peut forcer la signalisation avec
    put_flush_lpe() et put_flush_lmi(). C'est souvent inutile dans le
    modèle avec interruptions.
  - Dans tous les cas : toutes les entrées de LMI sont signalées (avec
    ou sans NOR)
  - Dans tous les cas : seules les entrées de LPE avec NOS sont signalées

  ON PEUT DONC PANACHER dans les deux modèles des émissions et réceptions
  avec et sans signalisation. La limitation est la suivante :
  dans le modèles avec interruptions, la signalisation s'accompagne
  systématiquement d'interruption en émission (mais pas en réception).
  Ca s'explique : en réception on a 2 flags : LMI pour la signalisation
  et NOR pour l'interruption. En émission on n'en a qu'un seul pour
  ces deux fonctionnalités : NOS. On aurait pu, dans le modèle à
  interruptions, signaler aussi les messages émis sans NOS, mais alors
  tous les messages émis auraient été signalés. En réception,
  l'utilisateur dispose des deux possibilités : signalisation ou pas,
  et interruption ou pas.


             signal.     Oui    Non
  interrupt
       oui               PUT    PUT
       non           PUT/PAPI   PUT

 */

/*
  PCIDDC1stRun nécessite :
  PUT_MODEL_INTERRUPT_DRIVEN, et ni PUT_OPTIMIZE, ni PUT_DONT_FLUSH_LPE
*/

#ifdef WITH_KLD
#define KLD_MODULE
#endif

#ifdef PCIDDC1stRun

#define PUT_MODEL_INTERRUPT_DRIVEN
#ifdef PUT_MODEL_POLLING
#undef PUT_MODEL_POLLING
#endif

#ifdef PUT_OPTIMIZE
#undef PUT_OPTIMIZE
#endif

#ifdef PUT_DONT_FLUSH_LPE
#undef PUT_DONT_FLUSH_LPE
#endif

#else /* PCIDDC2ndRun */

#define PUT_MODEL_INTERRUPT_DRIVEN

/* A METTRE SI LPE_SLOW, car plus logique : on perdra moins
   de CPU et le traitement "flush LPE" est alors inutile */
#define PUT_DONT_FLUSH_LPE

/*#define PUT_OPTIMIZE*/

#define LPE_SLOW
/*#define LPE_BURST*/ /* Points de synchro pour vidage de la LPE */
/* Avec un BX, le BURST ne corrige pas les pbs : irq en trop ET données
perdues. Même si on fait une pause de 100 ms à chaque irq en trop
et avec une taille (BURST_LIMIT) de burst valant 2.
Conclusion : le BURST ne sert à rien... */
#define LPE_BURST_LIMIT 8 /* Must be a power of 2 */
/* In fact, HSL_LPE_SIZE should simply be a multiple of LPE_BURST_LIMIT,
   no power of 2 necessary */
/* ATTENTION : AVEC LPE_SLOW, ON AJOUTE UN (NOS|LMP) A CHAQUE PAGE,
   DONC NE METTRE LMI QUE POUR LA DERNIERE PAGE DU MESSAGE, SINON
   ON AURA UN MI POUR CHAQUE PAGE INTERMEDIAIRE... */

#if defined(LPE_SLOW) && defined(LPE_BURST)
#error LPE_SLOW and LPE_BURST cannot be defined at the same time
#endif

#if defined(LPE_SLOW)
#ifndef PUT_DONT_FLUSH_LPE
#error PUT_DONT_FLUSH_LPE must be defined when using LPE_SLOW or LPE_BURST
#endif
#endif

#ifndef PUT_MODEL_INTERRUPT_DRIVEN
#define PUT_MODEL_POLLING
#endif

#endif

/*#define WARN_FOR_UNATTENDED_INTERRUPT*/

#include <sys/types.h>

#include "../MPC_OS/mpcshare.h"

#include "put.h"
#include "data.h"
#include "driver.h"

#include "cmem.h"

#include <sys/kernel.h>

#include <sys/malloc.h>
#include <sys/systm.h>

#include <sys/syslog.h>
#include <sys/errno.h>

#include <vm/vm.h>
#include <vm/vm_kern.h>

#ifdef _SMP_SUPPORT
#include <sys/lock.h>
#else
#include <vm/lock.h>
#endif
#include <vm/pmap.h>
#include <vm/vm_map.h>
#include <machine/pmap.h>

#include "/sys/i386/isa/isa_device.h"
#include "/sys/pci/pcivar.h"
#include "/sys/pci/pcireg.h"
#ifndef _SMP_SUPPORT
#include "/sys/pci/pcibus.h"
#endif

#ifdef _WITH_MONITOR
static node_t pnode2node[MAX_NODES];
#endif

#ifdef AVOID_LINK_ERROR
put_info_t put_info[MAX_PCI_INT];
#else
static put_info_t put_info[MAX_PCI_INT];
#endif

#ifdef AVOID_LINK_ERROR
interrupt_table_t *interrupt_table_LPE;
interrupt_table_t *interrupt_table_LMI;
u_long cptsignal_interrupt_table_LPE;
u_long cptsignal_interrupt_table_LMI;
#endif

static void put_interrupt_handler __P((void *));

#ifndef AVOID_LINK_ERROR
static int _put_add_entry __P((int, lpe_entry_t *));
#else
int _put_add_entry __P((int, lpe_entry_t *));
#endif

int hslgetlpe_ident;
int hslfreelpe_ident;

vm_offset_t opt_contig_space;
vm_size_t   opt_contig_size;
caddr_t     opt_contig_space_phys;
int         opt_slot;

vm_offset_t misalign_contig_space;
vm_size_t   misalign_contig_size;
caddr_t     misalign_contig_space_phys;
int         misalign_slot;
#ifdef PCIDDC2ndRun
boolean_t   misalign_in_use;
#endif

static boolean_t irq_registered = FALSE;

/* For software fix of hardware bug (Short Messages) */
static sm_emu_buf_t emu_buf_tab[128];
static int emu_buf_tab_cpt[128];

/*
 * For software fix of hardware bug : don't disable MASTER-ENABLE bit
 * until PCI-DDC is really started. This means we flush PCI-DDC interrupts
 * until the chip is fully initialized.
 */
static boolean_t flush_interrupts;

#ifdef AVOID_LINK_ERROR
static int avoid_cnt;
/* (stage == 0) <=> pas de problème
   (stage == 1) <=> demande de reveil de hslclient suite à EEP
   (stage == 2) <=> hslclient est réveillé
 */
int avoid_stage;
boolean_t avoid_MI_safe;
u_long avoid_v0;
lmi_entry_t *avoid_v0_entry;
lmi_entry_t *avoid_v0_entry_n;  /* next */
lmi_entry_t *avoid_v0_entry_nn; /* next next */
boolean_t avoid_simulate;
#endif


/************************************************************/

#ifdef AVOID_LINK_ERROR

static int
interrupt_distance(index0, index1)
                   int index0, index1;
{
  int distance;

  if (index0 > index1) {
    int tmp;
    tmp = index0;
    index0 = index1;
    index1 = tmp;
  }

  distance = MIN(index1 - index0, index0 + (1<<16) - index1);
  return distance;
}

static boolean_t
IDXA_INFSTRICT_IDXB(index0, index1)
                    int index0, index1;
{
  if (index0 < 16 &&
      index1 > (1<<16) - 16)
    index0 += 1<<16;

  if (index0 < index1)
    return TRUE;
  return FALSE;
}

static lpe_entry_t unused_lpe_entry;

static void
register_interrupt(table, index, fct, lpe_entry, use_lpe, arg0, arg1, arg2, arg3)
                   interrupt_table_t *table;
		   int index;
		   void (*fct)();
		   lpe_entry_t lpe_entry;
		   boolean_t use_lpe;
		   u_long arg0, arg1, arg2, arg3;
{
  u_long args[4] = { arg0, arg1, arg2, arg3 };
  int i;

  /* La table est vide ? */
  if (table->nentries == 0) {
    table->interrupt[0].index = index;
    table->interrupt[0].fct = fct;
    table->interrupt[0].lpe_entry = lpe_entry;
    table->interrupt[0].use_lpe = use_lpe;
    memcpy(table->interrupt[0].args,
	   args,
	   sizeof args);
    table->nentries = 1;
    return;
  }

  /* Est-on dans l'intervalle de la table ? */
  if (((table->interrupt[0].index <= table->interrupt[table->nentries - 1].index) &&
       (index >= table->interrupt[0].index &&
	index <= table->interrupt[table->nentries - 1].index)) ||
      ((table->interrupt[0].index > table->interrupt[table->nentries - 1].index) &&
       (index >= table->interrupt[0].index ||
	index <= table->interrupt[table->nentries - 1].index))) {
    /* OUI, on est dans l'intervalle de la table */

    for (i = 0; i < table->nentries; i++) {
      if (table->interrupt[i].index == index) {
	table->interrupt[i].fct = fct;
	table->interrupt[i].lpe_entry = lpe_entry;
	table->interrupt[i].use_lpe = use_lpe;
	memcpy(table->interrupt[i].args,
	       args,
	       sizeof args);
	table->nentries++;
	if (table->nentries == table->maxentries)
	  panic("register_interrupt: too many entries\n");
	return;
      }

      if (i != table->nentries - 1 &&
	  interrupt_distance(table->interrupt[i].index, index) < 16 &&
	  interrupt_distance(table->interrupt[i + 1].index, index) < 16 &&
	  IDXA_INFSTRICT_IDXB(table->interrupt[i].index, index) &&
	  IDXA_INFSTRICT_IDXB(index, table->interrupt[i + 1].index)) {
	bcopy(&table->interrupt[i + 1],
	      &table->interrupt[i + 2],
	      (table->nentries - (i + 1)) * sizeof(interrupt_entry_t));
	table->interrupt[i + 1].index = index;
	table->interrupt[i + 1].fct = fct;
	table->interrupt[i + 1].lpe_entry = lpe_entry;
	table->interrupt[i + 1].use_lpe = use_lpe;
	memcpy(table->interrupt[i + 1].args,
	       args,
	       sizeof args);
	table->nentries++;
	if (table->nentries == table->maxentries)
	  panic("register_interrupt: too many entries\n");
	return;
      }
    }
    /* NOTREACHED */
    panic("register_interrupt: NOTREACHED\n");
  }

  /* NON, on n'est pas dans l'intervalle de la table */

  if (interrupt_distance(index,
			 table->interrupt[0].index) <
      interrupt_distance(index,
			 table->interrupt[table->nentries - 1].index)) {
    bcopy(&table->interrupt[0],
	  &table->interrupt[1],
	  table->nentries * sizeof(interrupt_entry_t));
    table->interrupt[0].index = index;
    table->interrupt[0].fct = fct;
    table->interrupt[0].lpe_entry = lpe_entry;
    table->interrupt[0].use_lpe = use_lpe;
    memcpy(table->interrupt[0].args,
	   args,
	   sizeof args);
    table->nentries++;
    if (table->nentries == table->maxentries)
      panic("register_interrupt: too many entries\n");
    return;
  }

  table->interrupt[table->nentries].index = index;
  table->interrupt[table->nentries].fct = fct;
  table->interrupt[table->nentries].lpe_entry = lpe_entry;
  table->interrupt[table->nentries].use_lpe = use_lpe;
  memcpy(table->interrupt[table->nentries].args,
	 args,
	 sizeof args);
  table->nentries++;
  if (table->nentries == table->maxentries)
    panic("register_interrupt: too many entries\n");
}

static void
do_interrupts(table, index)
              interrupt_table_t *table;
	      int index;
{
  int i;
  u_long *cptsignalp;

  if (table->nentries == 0)
    return;

  if (table == interrupt_table_LPE)
    cptsignalp = &cptsignal_interrupt_table_LPE;
  else {
    if (table == interrupt_table_LMI)
      cptsignalp = &cptsignal_interrupt_table_LMI;
    else panic("do_interrupts: invalid table\n");
  }

  for (i = 0; i < table->nentries; i++) {
    if (*cptsignalp != table->interrupt[i].index)
      panic("do_interrupts: misordered table\n");
    if (++*cptsignalp == 65536)
      *cptsignalp = 0;

    if (table->interrupt[i].use_lpe == TRUE)
      ((void (*)(u_long, u_long, lpe_entry_t)) table->interrupt[i].fct)
	(table->interrupt[i].args[0],
	 table->interrupt[i].args[1],
	 table->interrupt[i].lpe_entry);
    else
      ((void (*)(u_long, u_long, u_long, u_long)) table->interrupt[i].fct)
	(table->interrupt[i].args[0],
	 table->interrupt[i].args[1],
	 table->interrupt[i].args[2],
	 table->interrupt[i].args[3]);

    if (table->interrupt[i].index == index)
      break;
  }

  if (i == table->nentries)
    panic("do_interrupts: index not found\n");

  if (++i == table->nentries) {
    table->nentries = 0;
    return;
  }

  bcopy(&table->interrupt[i],
	&table->interrupt[0],
	(table->nentries - i) * sizeof(interrupt_entry_t));
  table->nentries -= i;
}

#endif

/************************************************************/

/*
 ************************************************************
 * set_mode_read(minor, mode) : access HSL/Ethernet mode for a distant node.
 * minor : board number.
 * mode  : distant (p)node.
 *
 * return values : 0      : success.
 *                 ERANGE : node out of range.
 ************************************************************
 */

int
set_mode_read(minor, mode)
              int minor;
              set_mode_t *mode;
{
  mode->max_nodes = MAX_NODES;

  if (mode->node >= MAX_NODES)
    return ERANGE;

  mode->val =
    put_info[minor].remote_hsl[_PNODE2NODE(mode->node)];

  return 0;
}


/*
 ************************************************************
 * set_mode_write(minor, mode) : set HSL/Ethernet mode for a distant node.
 * minor : board number.
 * mode  : distant (p)node and access mode.
 *
 * return values : 0      : success.
 *                 ERANGE : node out of range.
 ************************************************************
 */

int
set_mode_write(minor, mode)
               int minor;
               set_mode_t *mode;
{
  if (mode->node >= MAX_NODES)
    return ERANGE;

  put_info[minor].remote_hsl[_PNODE2NODE(mode->node)] =
    mode->val;

  return 0;
}


/*
 ************************************************************
 * hsl_conf_read(minor, reg) : read a register in the configuration space.
 * minor : board number.
 * reg   : register to be read.
 *
 * return value : the register content or 0xFFFFFFFF if no board found.
 ************************************************************
 */

u_long
hsl_conf_read(minor, reg)
              int minor;
              u_long reg;
{
  int s;
  u_long val;
#if defined(LPE_SLOW) || defined(LPE_BURST)
  int i;
#endif

  if (put_info[minor].hsl_found == FALSE)
    return 0xFFFFFFFFUL;

  s = splhigh();

#if defined(LPE_SLOW) || defined(LPE_BURST)
  i = 0;
  do {
#endif

    /* It would be nicer to call pci_conf_read() */
#ifndef _SMP_SUPPORT
    val = getpcibus()->pb_read(put_info[minor].hsl_tag, reg);
#else
    val = pci_cfgread(&put_info[minor].hsl_probe, reg, 4);
#endif

#if defined(LPE_SLOW) || defined(LPE_BURST)
    if (!~val)
      log(LOG_ERR,
	  "Configuration read on register 0x%x: 0xffffffff - RETRYING(%d)\n",
	  (int) reg, i);
  } while (!~val && i++ < 10);
#endif

#ifdef AVOID_LINK_ERROR
  if (avoid_simulate == TRUE && reg == PCIDDC_STATUS)
    val |= PCIDDC_STATUS_EEP_ERROR;
#endif

  splx(s);

  return val;
}

/*
 ************************************************************
 * hsl_conf_write(minor, reg) : write a register in the configuration space.
 * minor : board number.
 * reg   : register to be written.
 ************************************************************
 */

void
hsl_conf_write(minor, reg, val)
               int minor;
               u_long reg;
               u_long val;
{
  int s;

  /* log(LOG_WARNING, "HSL_CONF_WRITE(0x%x) = 0x%x\n", reg, val); */

  if (put_info[minor].hsl_found == FALSE)
    return;

  s = splhigh();
  /* It would be nicer to call pci_conf_write() */
#ifndef _SMP_SUPPORT
  getpcibus()->pb_write(put_info[minor].hsl_tag, reg, val);
#else
  pci_cfgwrite(&put_info[minor].hsl_probe, reg, val, 4);
#endif

#ifdef AVOID_LINK_ERROR
  if (avoid_simulate == TRUE &&
      reg == PCIDDC_STATUS &&
      (val & PCIDDC_STATUS_EEP_ERROR))
    avoid_simulate = FALSE;
#endif
  splx(s);
}


/*
 ************************************************************
 * put_set_device(minor, tag) : register a PCI tag for a board.
 * minor : board number.
 * tag   : PCI tag.
 ************************************************************
 */

#ifndef _SMP_SUPPORT
void
put_set_device(minor, tag)
               int minor;
               pcici_t tag;
{
  put_info[minor].hsl_found = TRUE;
  put_info[minor].hsl_tag = tag;
}
#else
void
put_set_device(minor, probe)
               int minor;
               pcicfgregs probe;
{
  put_info[minor].hsl_found = TRUE;
  /* Which fields in probe should be updated ??? */
  probe.intpin   = pci_cfgread(&probe, PCIR_INTPIN,   1);
  probe.intline  = pci_cfgread(&probe, PCIR_INTLINE,  1);
  probe.lattimer = pci_cfgread(&probe, PCIR_LATTIMER, 1);
  put_info[minor].hsl_probe = probe;
}
#endif


/*
 ************************************************************
 * get_tables_info(minor, inf) : get informations about tables
 *                                   used by PCI-DDC.
 * minor : get info about this board.
 * inf   : feed this data structure with informations.
 ************************************************************
 */

void
get_tables_info(minor, inf)
                int minor;
                tables_info_t *inf;
{
  inf->lpe_virt = (caddr_t) put_info[minor].hsl_lpe;
  inf->lmi_virt = (caddr_t) put_info[minor].hsl_lmi;
  inf->lrm_virt = (caddr_t) put_info[minor].hsl_lrm;

  inf->lpe_phys = vtophys(inf->lpe_virt);
  inf->lmi_phys = vtophys(inf->lmi_virt);
  inf->lrm_phys = vtophys(inf->lrm_virt);

  inf->lpe_size = HSL_LPE_SIZE;
  inf->lmi_size = HSL_LMI_SIZE;
  inf->lrm_size = HSL_LRM_SIZE;
}


/*
 ************************************************************
 * put_init_SAP() : clean the data structures used by the interface.
 *
 * return value : 0 on success.
 ************************************************************
 */

int
put_init_SAP()
{
  int i;

  TRY("put_init_SAP")

  bzero(put_info,
	sizeof put_info);

  for (i = 0; i < MAX_PCI_INT; i++)
    put_info[i].hsl_contig_slot = -1;

  /************************************************************/

  opt_contig_size = OPTIONAL_CONTIG_RAM_SIZE;
  opt_slot        = cmem_getmem(opt_contig_size, "PUT");

  if (opt_slot < 0) {
    log(LOG_WARNING, "Can't allocate opt_contig_space\n");

    opt_contig_space      = NULL;
    opt_contig_space_phys = NULL;
    opt_contig_size       = 0;
  } else {
#ifdef DEBUG_HSL
    log(LOG_DEBUG,
	"allocation of opt_contig_space successful : %d bytes\n",
	opt_contig_size);
#endif

    opt_contig_space = cmem_virtaddr(opt_slot);
    opt_contig_space_phys = (caddr_t) cmem_physaddr(opt_slot);
  }

  /************************************************************/

  misalign_contig_size = 65536 + 8;
  misalign_slot        = cmem_getmem(misalign_contig_size, "MISALIGN");

  if (misalign_slot < 0) {
    log(LOG_WARNING, "Can't allocate misalign_contig_space\n");

    misalign_contig_space      = NULL;
    misalign_contig_space_phys = NULL;
    misalign_contig_size       = 0;
#ifdef PCIDDC2ndRun
    misalign_in_use            = FALSE;
#endif
  } else {
#ifdef DEBUG_HSL
    log(LOG_DEBUG,
	"allocation of misalign_contig_space successful : %d bytes\n",
	misalign_contig_size);
#endif

    misalign_contig_space      = cmem_virtaddr(misalign_slot);
    misalign_contig_space_phys = (caddr_t) cmem_physaddr(misalign_slot);
#ifdef PCIDDC2ndRun
    misalign_in_use            = FALSE;
#endif
  }

  /************************************************************/

  /* Software fix for Short Messages */
  bzero(emu_buf_tab,     sizeof emu_buf_tab);
  bzero(emu_buf_tab_cpt, sizeof emu_buf_tab_cpt);

  return 0;
}


/*
 ************************************************************
 * put_end_SAP() : close the PUT interface.
 * This interface can drive several physical boards.
 * If one of the physical board has not been correctly shut down
 * before the call to put_end_SAP(), it close the board now.
 *
 * return value : 0 on success.
 ************************************************************
 */

int
put_end_SAP()
{
  int i;

  TRY("put_end_SAP")

  for (i = 0; i < MAX_PCI_INT; i++)
    if (put_info[i].lpe) {
      log(LOG_WARNING, "put_end_SAP(): force put_end(minor = %d)\n", i);
      put_end(i);
    }

  if (opt_slot != -1)
    cmem_releasemem(opt_slot);

  if (misalign_slot != -1)
    cmem_releasemem(misalign_slot);

  for (i = 0; i < MAX_PCI_INT; i++)
    if (put_info[i].hsl_contig_slot != -1) {
      cmem_releasemem(put_info[i].hsl_contig_slot);
      put_info[i].hsl_contig_slot = -1;
      put_info[i].hsl_contig_space = 0;
      put_info[i].hsl_contig_space_backup = 0;
      put_info[i].hsl_contig_size = 0;
      put_info[i].hsl_contig_space_phys = 0;
    }

  return 0;
}


/*
 ************************************************************
 * put_init(minor, entries, node, routing_table) : initialization
 * of the PUT interface.
 * minor         : the board to initialize.
 * entries       : the number of entries in the LPE while in
 *                 emulator mode.
 * node          : the node number for this board.
 * routing_table : the R3 routing table.
 *
 * return values : 0      : success.
 *                 ENXIO  : not enough structures allocated at
 *                          compilation time to handle this board.
 *		   EINVAL : zero length LPE.
 *                 EBUSY  : this board already initialized.
 *                 ENOMEM : not enough memory.
 *                 EFAULT : routing_table is a bad address.
 ************************************************************
 */

int
put_init(minor, entries, node, routing_table)
         int    minor;
         u_long entries;
         node_t node;
         void  *routing_table;
{
  int s;
  lpe_entry_t *buf;
  int i;
  u_char rt[258];
  u_long irq_save;
  u_long lat_save;
  u_long val;
  int res;
  int cpt;
#ifdef DEBUG_HSL
  int irq;
#endif

  TRY("put_init")

#ifdef DEBUG_HSL
  log(LOG_DEBUG, "put_init(%d, %u, %d)\n",
      minor,
      (u_int) entries,
      node);
#endif

  if (minor < 0 || minor >= MAX_PCI_INT) return ENXIO;

#ifdef DEBUG_HSL
log(LOG_DEBUG,
    "put_init(): put_info[%d].lpe=%p\n",
    minor,
    put_info[minor].lpe);
#endif

  if (!entries) {
    log(LOG_WARNING,
	"put_init: zero length LPE\n");
    return EINVAL;
  }

  if (put_info[minor].lpe) {
    log(LOG_WARNING,
	"put_init: already allocated %d\n",
	minor);
    return EBUSY;
  }

  /* Software fix */
  flush_interrupts = TRUE;

#ifdef AVOID_LINK_ERROR
  avoid_cnt = 0;
  avoid_MI_safe = FALSE;
  avoid_stage = 0;
  avoid_v0 = 0L;
  avoid_v0_entry = avoid_v0_entry_n = avoid_v0_entry_nn = NULL;
  avoid_simulate = FALSE;
#endif

  /* Fetch the R3 routing table from the user space */
  if (routing_table != NULL) {
    res = copyin(routing_table, rt, sizeof rt);
    if (res)
      return EFAULT;
  }
  else /* Loading garbage in the R3 is not what we want... */
    bzero(rt, sizeof rt);

  /* previous use was not clean */
  if (put_info[minor].hsl_contig_space)
    return EBUSY;

  /* Allocate a LPE for the emulator mode */

  MALLOC(buf,
	 lpe_entry_t *,
	 entries * sizeof(lpe_entry_t),
	 M_DEVBUF,
	 M_WAITOK);

  if (buf == NULL) {
    log(LOG_ERR,
	"put_init(): MALLOC() returned NULL\n");
    return ENOMEM;
  }

  put_info[minor].lpe      = buf;
  put_info[minor].nentries = entries;
  put_info[minor].lpe_high = 0;
  put_info[minor].lpe_low  = 0;
  put_info[minor].node = node;
  put_info[minor].first_space = NULL;

#ifdef AVOID_LINK_ERROR
  cptsignal_interrupt_table_LPE = 0;
  MALLOC(interrupt_table_LPE,
	 interrupt_table_t *,
	 sizeof(interrupt_table_t) +
	 HSL_LPE_SIZE * sizeof(interrupt_entry_t),
	 M_DEVBUF,
	 M_WAITOK);
  if (!interrupt_table_LPE) {
    FREE(put_info[minor].lpe,
	 M_DEVBUF);
    put_info[minor].lpe = NULL;
    log(LOG_ERR,
	"put_init(): MALLOC() LPE INTR returned NULL\n");
    return ENOMEM;
  }

  cptsignal_interrupt_table_LMI = 0;
  MALLOC(interrupt_table_LMI,
	 interrupt_table_t *,
	 sizeof(interrupt_table_t) +
	 HSL_LMI_SIZE * sizeof(interrupt_entry_t),
	 M_DEVBUF,
	 M_WAITOK);
  if (!interrupt_table_LMI) {
    FREE(put_info[minor].lpe,
	 M_DEVBUF);
    put_info[minor].lpe = NULL;
    FREE(interrupt_table_LPE,
	 M_DEVBUF);
    log(LOG_ERR,
	"put_init(): MALLOC() LMI INTR returned NULL\n");
    return ENOMEM;
  }
#endif

  interrupt_table_LPE->maxentries = HSL_LPE_SIZE;
  interrupt_table_LMI->maxentries = HSL_LMI_SIZE;
  interrupt_table_LPE->nentries = interrupt_table_LMI->nentries = 0;

  /* After this line, we MUST terminate correctly the function call,
     because put_info[minor].lpe is allocated, and this
     event is used to control whether the initialization has been performed
     or not (see hslclient.c : if put_init() returns an error,
     it closes PUT and retries) */
  /* Another way to be safe would be to deallocate every allocated resources
     before returning on error */

  for (i = 0; i < MAX_SAP; i++) {
    put_info[minor].IT_sent[i]     = NULL;
    put_info[minor].IT_received[i] = NULL;
    bzero(put_info[minor].mi_space + i,
	  sizeof(struct _mi_space));
  }

  /* Allocate data structures for one HSL board */

  /* compute the size of the physical contig. memory we need to allocate */
  put_info[minor].hsl_contig_size =
    sizeof(*put_info[minor].hsl_lpe) +
    sizeof(*put_info[minor].hsl_lmi) +
    sizeof(*put_info[minor].hsl_lrm);
  
  /* Check if we have already reserved a slot */
  if (!put_info[minor].hsl_contig_space_backup) {
    /* This is the first time we ask for a slot */

    /* Ask for a slot of contig. memory */
    put_info[minor].hsl_contig_slot =
      cmem_getmem(put_info[minor].hsl_contig_size, "PCIDDC");
    if (put_info[minor].hsl_contig_slot < 0) {
      put_info[minor].hsl_contig_space = NULL;
      /* We should deallocate previously allocated resources like the
	 LPE for the emulator mode...*/
      return ENOMEM;
    }

    /* Get info about the slot */
    put_info[minor].hsl_contig_space =
      cmem_virtaddr(put_info[minor].hsl_contig_slot);
    put_info[minor].hsl_contig_space_phys =
      (caddr_t) cmem_physaddr(put_info[minor].hsl_contig_slot);
#ifdef DEBUG_HSL
    log(LOG_DEBUG, "put_init: hsl_contig_size = 0x%x\n",
	put_info[minor].hsl_contig_size);
    log(LOG_DEBUG,
	"put_init: contig_space = 0x%x\n",
	put_info[minor].hsl_contig_space);
    log(LOG_DEBUG,
	"put_init: hsl_contig_space_phys = 0x%x\n",
	(u_int) put_info[minor].hsl_contig_space_phys);
#endif
    if (!put_info[minor].hsl_contig_space)
      /* We should deallocate previous allocated resources like the
	 LPE for the emulator mode... */
      return ENOMEM;
    put_info[minor].hsl_contig_space_backup = put_info[minor].hsl_contig_space;
  } else {
    /* We already have an allocated slot */
    put_info[minor].hsl_contig_space = put_info[minor].hsl_contig_space_backup;
#ifdef DEBUG_HSL
    log(LOG_DEBUG,
	"put_init: REUSE of hsl_contig_space_backup\n");
#endif
  }

  /*
   * Allocate the basic tables.
   * The LPE and LMI tables must be aligned on a multiple of 16 bytes.
   * Thus, the order of allocation must be this one : LPE, LMI and LRM.
   */
  put_info[minor].hsl_lpe = (void *) put_info[minor].hsl_contig_space;
  put_info[minor].hsl_lmi =
    (void *) put_info[minor].hsl_lpe + sizeof(*(put_info[minor].hsl_lpe));
  put_info[minor].hsl_lrm =
    (void *) put_info[minor].hsl_lmi + sizeof(*(put_info[minor].hsl_lmi));

  put_info[minor].lpe_softnew_virt =
    put_info[minor].lpe_new_virt =
    put_info[minor].lpe_current_virt =
    (lpe_entry_t *) put_info[minor].hsl_lpe;

  put_info[minor].lmi_current_virt =
    (lmi_entry_t *) put_info[minor].hsl_lmi;

  put_info[minor].lpe_loaded = FALSE;

  bzero((void *) put_info[minor].hsl_contig_space,
	put_info[minor].hsl_contig_size);

  /* Register interrupt handler */

  if (irq_registered == FALSE && put_info[minor].hsl_found == TRUE) {
#ifdef DEBUG_HSL
#ifdef _SMP_SUPPORT
#define	PCI_INTERRUPT_LINE_MASK	0x000000ff
#define	PCI_INTERRUPT_LINE_EXTRACT(x) ((((x) & PCI_INTERRUPT_LINE_MASK) >> 0) & 0xff)
#endif
    /* Get irq line from configuration space */
    irq = PCI_INTERRUPT_LINE_EXTRACT(hsl_conf_read(minor,
						   PCI_INTERRUPT_REG));
    log(LOG_DEBUG,
	"Register Interrupt Handler : irq=0x%x\n",
	irq);
#endif

#ifndef _SMP_SUPPORT
    res = pci_map_int(put_info[minor].hsl_tag,
		      put_interrupt_handler,
		      (void *) minor,
		      &net_imask);
#else
#ifndef _SMP_KERNEL
    res = pci_map_int(&put_info[minor].hsl_probe,
		      put_interrupt_handler,
		      (void *) minor,
		      &net_imask);
#else
    {
      extern void (*mpc_dynamic_handler)(void *);
      mpc_dynamic_handler = put_interrupt_handler;
      res = 1;
    }
#endif
#endif

    if (!res) {
      log(LOG_DEBUG,
	  "Retrying interrupt registration\n");
#ifndef _SMP_SUPPORT
      res = pci_map_int(put_info[minor].hsl_tag,
			put_interrupt_handler,
			(void *) minor,
			&net_imask);
#else
      res = pci_map_int(&put_info[minor].hsl_probe,
			put_interrupt_handler,
			(void *) minor,
			&net_imask);
#endif
    }

    if (res)
      irq_registered = TRUE;
    else
      log(LOG_ERR,
	  "Failed to register irq : %d\n",
	  res);
  } else
    log(LOG_ERR,
	"IRQ already registered\n");

  /* Initialization of the PCI-DDC chip */

#define PCIDDC_VIRT2PHYS(X) (((u_long) (X)) - \
			     ((u_long) put_info[minor].hsl_contig_space) + \
			     (u_long) put_info[minor].hsl_contig_space_phys)

  /* Do the last job many times to fix a hardware bug in the reset procedure */
  /*
   * 3 times seems to be sufficient : like women, the behaviour
   * of our chip is a bit strange with this reset problem, and we want
   * to be sure the state of PCI-DDC is correct.
   */
  s = splhigh();
  for (cpt = 3; cpt >= 0; cpt--) {

    /* Remember values lost at soft reset time */
    irq_save = hsl_conf_read(minor,
			     PCI_INTERRUPT_REG);
#ifdef _SMP_SUPPORT
#define PCI_HEADER_MISC	0x0c
#endif
    lat_save = hsl_conf_read(minor,
			     PCI_HEADER_MISC);

    /* Perform a soft reset */
    hsl_conf_write(minor,
		   PCIDDC_COMMAND,
		   PCIDDC_SOFT_RESET);

    /*
     * Restore the lost values
     * (WARNING : WE ASSUME HERE THAT pci_map_int() DID NOT CHANGE THE
     * INTERRUPT LINE SELECTED BY THE BIOS...)
     */
    hsl_conf_write(minor,
		   PCI_INTERRUPT_REG,
		   irq_save);
    hsl_conf_write(minor,
		   PCI_HEADER_MISC,
		   lat_save);

    /* Set latency timer : the BIOS has set a value, we put
       the highest value (this can improve a lot the performances) */
    hsl_conf_write(minor,
		   PCI_HEADER_MISC,
		   0xFF00 | lat_save);

    /* hsl_conf_write(minor, PCIDDC_STATUS_MASK_REGISTER, 0); */

    /* Clear the interrupt mask */
    hsl_conf_write(minor,
		   PCIDDC_STATUS_MASK_REGISTER,
		   0UL);

    /* Set LPE max address */
    hsl_conf_write(minor,
		   PCIDDC_LPE_MAX,
		   PCIDDC_VIRT2PHYS(((lpe_entry_t *) put_info[minor].hsl_lpe) +
				    HSL_LPE_SIZE - 1));

    /* Set LPE current address */
    hsl_conf_write(minor,
		   PCIDDC_LPE_CURRENT,
		   PCIDDC_VIRT2PHYS(put_info[minor].hsl_lpe));

    /* Set LPE new address */
    hsl_conf_write(minor,
		   PCIDDC_LPE_NEW,
		   PCIDDC_VIRT2PHYS(put_info[minor].hsl_lpe));

    /* Set LPE base address */
    hsl_conf_write(minor,
		   PCIDDC_LPE_BASE,
		   PCIDDC_VIRT2PHYS(put_info[minor].hsl_lpe));

    /* Set LMI max address */
    hsl_conf_write(minor,
		   PCIDDC_LMI_MAX,
		   PCIDDC_VIRT2PHYS(((lmi_entry_t *) put_info[minor].hsl_lmi) +
				    HSL_LMI_SIZE - 1));

    /* Set LMI current address */
    hsl_conf_write(minor,
		   /* Due to a bug in PCI-DDC,
		      LMI_NEW & LMI_CURRENT are reverted as
		      they are written... */
		   /* PCIDDC_LMI_CURRENT */
		   PCIDDC_LMI_NEW,
		   PCIDDC_VIRT2PHYS(put_info[minor].hsl_lmi));

    /* Set LMI new address */
    hsl_conf_write(minor,
		   /* Due to a bug in PCI-DDC,
		      LMI_NEW & LMI_CURRENT are reverted as
		      they are written... */
		   /* PCIDDC_LMI_NEW */
		   PCIDDC_LMI_CURRENT,
		   PCIDDC_VIRT2PHYS(put_info[minor].hsl_lmi));

    /* Set LMI base address */
    hsl_conf_write(minor,
		   PCIDDC_LMI_BASE,
		   PCIDDC_VIRT2PHYS(put_info[minor].hsl_lmi));

    /* Set LRM base address */
    hsl_conf_write(minor,
		   PCIDDC_LRM_BASE,
		   PCIDDC_VIRT2PHYS(put_info[minor].hsl_lrm));

    /* Feed R3 with the routing table */
    if (routing_table != NULL && cpt == 0)
      for (i = sizeof(rt) - 1; i >= 0; i--) {
	/* log(LOG_DEBUG, "[%d] = 0x%x\n", i, rt[i]); */

	hsl_conf_write(minor,
		       PCIDDC_R3_DOUT,
		       rt[i]);
      }

    /* Bootstrap PCI-DDC */

    /* Not very useful */
    hsl_conf_write(minor,
		   PCIDDC_COMMAND,
		   (8<<PCIDDC_TXRX_BALANCE_START)

#ifdef PCIDDC1stRun
		   /* MPL is set to 1 to avoid a hardware race condition */
		   | (1<<PCIDDC_MAX_PACKET_LENGTH_START)
#else
		   /* Do nothing : MPL is set to 0 (means MAX) */
#endif

		   /* | PCIDDC_LOOPBACK (Loopback in PCI-DDC) */ /* |
		      PCIDDC_LRM_ENABLE <- LRM IS BROKEN */);         /* 4 */

    /* Initialize the Tx module */
    hsl_conf_write(minor,
		   PCIDDC_COMMAND,
		   (8<<PCIDDC_TXRX_BALANCE_START)

#ifdef PCIDDC1stRun
		   /* MPL is set to 1 to avoid a hardware race condition */
		   | (1<<PCIDDC_MAX_PACKET_LENGTH_START)
#else
		   /* Do nothing : MPL is set to 0 (means MAX) */
#endif

		   /* | PCIDDC_LOOPBACK (Loopback in PCI-DDC) */ /* |
		      PCIDDC_LRM_ENABLE <- LRM IS BROKEN */ |
		   PCIDDC_Tx_INIT);            /* 3 */

    /* Initialize the R3 module */
    hsl_conf_write(minor,
		   PCIDDC_COMMAND,
		   (8<<PCIDDC_TXRX_BALANCE_START)

#ifdef PCIDDC1stRun
		   /* MPL is set to 1 to avoid a hardware race condition */
		   | (1<<PCIDDC_MAX_PACKET_LENGTH_START)
#else
		   /* Do nothing : MPL is set to 0 (means MAX) */
#endif

		   /* | PCIDDC_LOOPBACK (Loopback in PCI-DDC) */ /* |
		      PCIDDC_LRM_ENABLE <- LRM IS BROKEN */ |
		   PCIDDC_Tx_INIT |
		   PCIDDC_R3_INIT);            /* 2 */

    /* Initialize the Rx module */
    hsl_conf_write(minor,
		   PCIDDC_COMMAND,
		   (8<<PCIDDC_TXRX_BALANCE_START)

#ifdef PCIDDC1stRun
		   /* MPL is set to 1 to avoid a hardware race condition */
		   | (1<<PCIDDC_MAX_PACKET_LENGTH_START)
#else
		   /* Do nothing : MPL is set to 0 (means MAX) */
#endif

		   /* | PCIDDC_LOOPBACK (Loopback in PCI-DDC) */ /* |
		      PCIDDC_LRM_ENABLE <- LRM IS BROKEN */ |
		   PCIDDC_Tx_INIT |
		   PCIDDC_R3_INIT |
		   PCIDDC_Rx_INIT);            /* 1 */
  }
  splx(s);

  /* Start the game */

  /*
   * Set the interrupt mask : do not mask anything but the link status
   * and PCIDDC_STATUS_R3_STATUS_ERROR.
   */
  hsl_conf_write(minor,
		 PCIDDC_STATUS_MASK_REGISTER,
#ifdef PUT_MODEL_INTERRUPT_DRIVEN
		 PCIDDC_STATUS_PAGE_TRANSMITTED |
		 PCIDDC_STATUS_PAGE_RECEIVED |
#endif
		 PCIDDC_STATUS_EEP_ERROR |
		 PCIDDC_STATUS_CRC_HEADER_ERROR |
		 PCIDDC_STATUS_CRC_DATA_ERROR |
		 PCIDDC_STATUS_EP_ERROR |
/*#ifndef AVOID_LINK_ERROR*/
		 PCIDDC_STATUS_TIMEOUT_ERROR |
/*#endif*/
		 PCIDDC_STATUS_PAGE_TRANSMITTED_OF_ERROR |
		 PCIDDC_STATUS_PAGE_RECEIVED_OF_ERROR |
		 PCIDDC_STATUS_SENT_PACKET_OF_ERROR |
		 PCIDDC_STATUS_RECEIVED_PACKET_OF_ERROR |
		 PCIDDC_STATUS_ILLEGAL_LRM_ACCESS_ERROR |
/*#ifndef AVOID_LINK_ERROR*/
		 PCIDDC_STATUS_LMI_OF_ERROR |
/*#endif*/
		 /* PCIDDC_STATUS_R3_STATUS_ERROR | */
		 PCIDDC_STATUS_R3_STATUS_MASK);

  /* Check parity on addresses and data */
  hsl_conf_write(minor,
		 PCI_COMMAND_STATUS_REG,
		 PCI_COMMAND_MASTER_ENABLE |
		 PCI_COMMAND_SERR_ENABLE /* |
                 PCI_COMMAND_PARITY_ENABLE */ ); /* Ignition... */

  /* Buy a PlayStation if it doesn't work... (it's more funny) */

  /* Initialize pnode to node translation table */
#ifdef _WITH_MONITOR
  for (i = 0; i < MAX_NODES; i++)
    pnode2node[i] = i;
#endif

  /* Initialize the use of the HSL network */

  val = hsl_conf_read(minor,
		      PCIDDC_STATUS);
  
  if (routing_table != NULL &&
      irq_registered == TRUE &&
      !(val & (PCIDDC_STATUS_EEP_ERROR |
	       PCIDDC_STATUS_CRC_HEADER_ERROR |
	       PCIDDC_STATUS_CRC_DATA_ERROR |
	       PCIDDC_STATUS_EP_ERROR |
	       PCIDDC_STATUS_TIMEOUT_ERROR |
	       PCIDDC_STATUS_R3_STATUS_ERROR |
	       PCIDDC_STATUS_PAGE_TRANSMITTED_OF_ERROR |
	       PCIDDC_STATUS_PAGE_RECEIVED_OF_ERROR |
	       PCIDDC_STATUS_SENT_PACKET_OF_ERROR |
	       PCIDDC_STATUS_RECEIVED_PACKET_OF_ERROR |
	       PCIDDC_STATUS_ILLEGAL_LRM_ACCESS_ERROR |
	       PCIDDC_STATUS_LMI_OF_ERROR))) {
#ifdef DEBUG_HSL
    log(LOG_DEBUG, "PUT: startup HSL mode\n");
#endif
    for (i = 0; i < MAX_NODES; i++)
      put_info[minor].remote_hsl[i] = TRUE;
  } else {
#ifdef DEBUG_HSL
    log(LOG_DEBUG, "PUT: startup Ethernet mode\n");
#endif
  }

  /* Software fix */
  /* Wait 1 sec and stop ignoring interrupts. */
  tsleep(&flush_interrupts, HSLPRI | PCATCH, "WAITIRQ", hz);
  flush_interrupts = FALSE;

#ifdef DEBUG_HSL
  log(LOG_DEBUG, "PUT_INIT COMPLETED\n");
#endif

  ADD_EVENT(TRACE_INIT_PUT);
  return 0;
}


/*
 ************************************************************
 * put_end(minor) : shut down one board handled by the PUT interface.
 * minor : board to shut down.
 *
 * return values : 0      : success.
 *                 ENXIO  : not enough structures allocated at
 *                          compilation time to handle this board.
 *                 ENOENT : this board is already down.
 ************************************************************
 */
                 
int
put_end(minor)
        int minor;
{
  int i;

  TRY("put_end")

#ifdef DEBUG_HSL
  log(LOG_DEBUG,
      "put_end(%d)\n",
      minor);
#endif

  if (minor < 0 || minor >= MAX_PCI_INT)
    return ENXIO;
  if (!put_info[minor].lpe)
    return ENOENT;

  /* Use Ethernet instead of HSL */
  for (i = 0; i < MAX_NODES; i++)
    put_info[minor].remote_hsl[i] = FALSE;

  /* Unmap pci interrupt */
  /* Note that pci_unmap_int() does NOTHING under FreeBSD-3.0
     (not supported function) */
  if (irq_registered == TRUE) {
#ifndef _SMP_SUPPORT
    pci_unmap_int(put_info[minor].hsl_tag);
#else
#ifndef _SMP_KERNEL
    pci_unmap_int(&put_info[minor].hsl_probe);
#else
    {
      extern void (*mpc_dynamic_handler)(void *);
      mpc_dynamic_handler = NULL;
    }
#endif
#endif
    irq_registered = FALSE;
  }

  FREE(put_info[minor].lpe,
       M_DEVBUF);

#ifdef AVOID_LINK_ERROR
  FREE(interrupt_table_LPE,
       M_DEVBUF);

  FREE(interrupt_table_LMI,
       M_DEVBUF);
#endif

  bzero(put_info + minor,
	((char *) &put_info[minor].hsl_found) -
	((char *) &put_info[minor].lpe));

  put_info[minor].hsl_contig_space = 0;

  ADD_EVENT(TRACE_END_PUT);
  return 0;
}


/*
 ************************************************************
 * put_get_node(minor) : get the (p)node affected to a board.
 * minor : board number.
 *
 * return values : node number on success.
 *                 -1 on error (board not initialized).
 ************************************************************
 */

int
put_get_node(minor)
             int minor;
{
#ifdef _WITH_MONITOR
  int i;
#endif

  TRY("put_get_node")

#ifdef DEBUG_HSL
  log(LOG_DEBUG,
      "put_get_node(%d)\n",
      minor);
#endif

  if (minor < 0 || minor >= MAX_PCI_INT)
    return -1;
  if (!put_info[minor].lpe)
    return -1;

#ifndef _WITH_MONITOR
  return put_info[minor].node;
#else
  for (i = 0; i < MAX_NODES; i++)
    if (pnode2node[i] == put_info[minor].node)
      return i;
  return -1;
#endif
}


/*
 ************************************************************
 * put_get_mi_start(minor, sap) : get the first mi allocated
 * for this board.
 * minor : the board.
 * sap   : the registration number affected to the user of this
 *         interface.
 *
 * return values : the required MI.
 ************************************************************
 */

mi_t
put_get_mi_start(minor, sap)
                 int minor;
                 int sap;
{
  /* TRY("put_get_mi_start") */

  return put_info[minor].mi_space[sap].mi_start;
}


/*
 ************************************************************
 * put_get_lpe_high(minor) : get a pointer to the current entry
 * the board is working on.
 * minor : the board number.
 *
 * return values : the required pointer.
 ************************************************************
 */

long
put_get_lpe_high(minor)
                 int minor;
{
  TRY("put_get_lpe_high")

  return put_info[minor].lpe_high;
}


/*
 ************************************************************
 * put_get_lpe_free(minor) : the number of free entries in the LPE.
 * minor : the board number.
 *
 * return values : the number required.
 ************************************************************
 */

long
put_get_lpe_free(minor)
                 int minor;
{
  long ret1, ret2;

  TRY("put_get_lpe_free")

  /* Compute number of free entries in the Ethernet mode LPE */
  ret1 = (put_info[minor].lpe_high < put_info[minor].lpe_low) ?
    (put_info[minor].lpe_low - put_info[minor].lpe_high - 1) :
    (put_info[minor].nentries + put_info[minor].lpe_low -
     put_info[minor].lpe_high - 1);

    /*
     * Software fix of hardware bug : short messages require 8 entries
     * in the LPE.
     */
  ret1 -= 7L;
  if (ret1 < 0) ret1 = 0;

  /* Compute number of free entries in the HSL mode LPE */
  ret2 = (put_info[minor].lpe_current_virt <=
	  put_info[minor].lpe_softnew_virt) ?
    (HSL_LPE_SIZE -
     (put_info[minor].lpe_softnew_virt - put_info[minor].lpe_current_virt)
     - 1) :
    ((put_info[minor].lpe_current_virt - put_info[minor].lpe_softnew_virt)
     - 1);

    /*
     * Software fix of hardware bug : short messages require 8 entries
     * in the LPE.
     */
  ret2 -= 7L;

#ifdef AVOID_LINK_ERROR
  /* On diminue ret2 pour toujours avoir de libre :
     - 2 entrées pour 2 message "vide" afin de s'assurer qu'il
     y a toujours au moins 2 message après l'index v0 dans la LPE,
     et juste avant les messages répétés.
     - 1 entrée pour l'insertion d'un marqueur juste avant les
     messages répétés.
     - 2*17 entrées pour 2 messages qu'on rajoutera à la LPE quand
     on detectera qu'ils ont potentiellement été perdus.
  */
  ret2 -= 3L + 34L;

  /* Imposer un no-man's-land de 16384 pages pour être sûr que
     les pages qui viennent d'être envoyées depuis peu ne sont
     pas écrasées par des pages qu'on voudrait envoyer. Ca nous
     permet d'avoir suffisamment de temps pour faire la recherche
     des deux pages potentiellement pas envoyées. */
  ret2 -= 16384;
#endif

  if (ret2 < 0) ret2 = 0;

  return MIN(ret1, ret2);
}


/*
 ************************************************************
 * put_register_SAP(minor, send, received) : register a user of
 * the PUT interface for one board.
 * minor    : the board number.
 * send     : the interrupt procedure for data sent.
 * received : the interrupt procedure for data received.
 *
 * NOTE that when fixing misaligned buffers hardware bug,
 * we alterate the PRSA and PLSA values in the LPE entries.
 * Then, the lpe_entry_t parameter of the `sent()' callback function
 * may have invalid PRSA/PLSA values.
 *
 * return values : -1 on error.
 *                 the registration number for this SAP on success.
 ************************************************************
 */

int
put_register_SAP(minor, sent, received)
                 int minor;
                 void (*sent) __P((int, mi_t, lpe_entry_t));
                 void (*received) __P((int, mi_t, u_long, u_long));
{
  int i;

  TRY("put_register_SAP")

#ifdef DEBUG_HSL
  log(LOG_DEBUG,
      "put_register_SAP(%d, %p, %p)\n",
      minor,
      sent,
      received);
#endif

  if (minor < 0 || minor >= MAX_PCI_INT)
    return -1;
  if (!put_info[minor].lpe)
    return -1;

  for (i = 0; i < MAX_SAP; i++)
    if (put_info[minor].in_use[i] == FALSE) {
      put_info[minor].IT_sent[i]     = sent;
      put_info[minor].IT_received[i] = received;
      put_info[minor].in_use[i]      = TRUE;
      return i;
    }

  return -1;
}


/*
 ************************************************************
 * put_unregister_SAP(minor, id) : unregister a user of the PUT
 * interface.
 * minor : the board number.
 * id    : the registration number for the SAP.
 *
 * return values : ENXIO  : not enough structures allocated at
 *                          compilation time to handle this board.
 *                 ENOENT : this board is not initialized.
 *                 EBADF  : id out of range.
 ************************************************************
 */

int
put_unregister_SAP(minor, id)
                 int minor, id;
{
  TRY("put_unregister_SAP")

#ifdef DEBUG_HSL
  log(LOG_DEBUG,
      "put_unregister_SAP(%d, %d)\n",
      minor,
      id);
#endif

  if (minor < 0 || minor >= MAX_PCI_INT)
    return ENXIO;
  if (!put_info[minor].lpe)
    return ENOENT;

  if (id < 0 ||
      id >= MAX_SAP ||
      put_info[minor].in_use[id] == FALSE)
    return EBADF;

  put_info[minor].IT_sent[id]     = NULL;
  put_info[minor].IT_received[id] = NULL;
  put_info[minor].in_use[id]      = NULL;

  /* Update the chain */

  if (put_info[minor].mi_space[id].in_use == TRUE) {
    /* Update first_space pointer */
    if (put_info[minor].mi_space + id == put_info[minor].first_space)
      put_info[minor].first_space =
	put_info[minor].mi_space[id].next;

    if (put_info[minor].mi_space[id].prev)
      put_info[minor].mi_space[id].prev->next =
	put_info[minor].mi_space[id].next;

    if (put_info[minor].mi_space[id].next)
      put_info[minor].mi_space[id].next->prev =
	put_info[minor].mi_space[id].prev;

    put_info[minor].mi_space[id].in_use = FALSE;
  }

  return 0;
}

/*
 ************************************************************
 * put_attach_mi_range(minor, sap, range) : ask for the attribution
 * of a range of MI.
 * minor : the board number.
 * sap   : the registration number for this SAP.
 * range : the size of the range.
 *
 * Note that the size MUST be a power of 2.
 * The first MI of the allocated range will be a multiple of the range
 * to simplify the work of the user of this interface.
 *
 * return values : 0      : success.
 *                 ENXIO  : not enough structures allocated at
 *                          compilation time to handle this board.
 *                 ENOENT : this board is not initialized.
 *                 EBUSY  : this SAP already has an allocated set
 *                          of MI.
 *                 E2BIG  : range is too big to be allocated, it should
 *                          be reduced or a new compilation of the
 *                          PUT interface should be required after
 *                          having increased the constant MI_SIZE.
 *                 EINVAL : range is neither null, nor a power of 2.
 *                 ENOMEM : not enough MI left.
 ************************************************************
 */

int
put_attach_mi_range(minor, sap, range)
                 int minor;
                 int sap;
                 u_long range;
{
  struct _mi_space *space;
  int i;
  int bits;

  TRY("put_attach_mi_range")

#ifdef DEBUG_HSL
  log(LOG_DEBUG,
      "put_attach_mi_range(%d, %d, %d)\n",
      minor,
      sap,
      (int) range);
#endif

  if (minor < 0 || minor >= MAX_PCI_INT)
    return ENXIO;

  if (!put_info[minor].lpe)
    return ENOENT;

  if (put_info[minor].in_use[sap] == FALSE)
    return EBADF;

  if (put_info[minor].mi_space[sap].in_use)
    return EBUSY;

  if (!range)
    return 0;

  /* Check that range is a power of 2 */
  bits = 0;
  for (i = 0; i < 32; i++)
    if (range & (1<<i)) bits++;
  if (bits != 1)
    return EINVAL;

  if (!put_info[minor].first_space) {
    if (range >= (1<<MI_SIZE))
      return E2BIG;
    put_info[minor].mi_space[sap].in_use   = TRUE;
    put_info[minor].mi_space[sap].mi_start = 0;
    put_info[minor].mi_space[sap].mi_size  = range;
    put_info[minor].mi_space[sap].prev     = NULL;
    put_info[minor].mi_space[sap].next     = NULL;
    put_info[minor].first_space = put_info[minor].mi_space + sap;
    return 0;
  }

  space = put_info[minor].first_space;
  while (space) {
    mi_t start;

    start = ((space->mi_start + space->mi_size - 1) & ~(range - 1)) + range;
    if (start >= (1<<MI_SIZE))
      return ENOMEM;

    if (!space->next) {
      put_info[minor].mi_space[sap].in_use   = TRUE;
      put_info[minor].mi_space[sap].mi_start = start;
      put_info[minor].mi_space[sap].mi_size  = range;
      put_info[minor].mi_space[sap].prev     = space;
      put_info[minor].mi_space[sap].next     = NULL;
      space->next = put_info[minor].mi_space + sap;
      return 0;
    }

    if ((start + range) <= space->next->mi_start) {
      put_info[minor].mi_space[sap].in_use   = TRUE;
      put_info[minor].mi_space[sap].mi_start = start;
      put_info[minor].mi_space[sap].mi_size  = range;
      put_info[minor].mi_space[sap].prev     = space;
      put_info[minor].mi_space[sap].next     = space->next;
      space->next->prev = put_info[minor].mi_space + sap;
      space->next = put_info[minor].mi_space + sap;
      return 0;
    }

    space = space->next;
  }

  return ENOMEM;
}


/*
 ************************************************************
 * bstore_phys(src_phys, dst_virt, size) : buffer copy from physical
 * space to virtual space.
 * src_phys : physical address of source buffer.
 * dst_virt : virtual address of dest buffer, in kernel space.
 *
 * NOTE THAT THIS FUNCTION MUST BE CALLED AT splhigh PROCESSOR LEVEL.
 ************************************************************
 */

#ifdef PCIDDC1stRun

static void
bstore_phys(src_phys, dst_virt, size)
            caddr_t src_phys;
	    caddr_t dst_virt;
	    size_t  size;
{
  caddr_t phys, phys_max;
  size_t  len;

  /* log(LOG_ERR, "STARTING BSTORE\n"); */

  phys_max = src_phys + size;
  for (phys = src_phys;
       phys < phys_max;
       phys = (caddr_t) trunc_page((unsigned) (phys + PAGE_SIZE))) {

    /* log(LOG_ERR, "BSTORE %p\n", phys); */

    len = MIN((caddr_t) trunc_page((unsigned) (phys + PAGE_SIZE)), phys_max)
          - phys;

    *(int *) CMAP1 = PG_V | PG_RW | trunc_page((unsigned) phys);

    bcopy((caddr_t) (CADDR1 + ((int) phys & PAGE_MASK)),
	  dst_virt + (phys - src_phys),
	  len);

    *(int *) CMAP1 = 0;
    invltlb();
  }
}

#endif


/*
 ************************************************************
 * put_add_entry(minor, entry) : add an entry to the LPE.
 * This function corrects the bogus handling of Short Messages and
 * Misaligned pages in PCIDDC 1st Run.
 * minor : board number.
 * entry : entry to add.
 *
 * return values : 0      : success.
 *                 ENXIO  : not enough structures allocated at
 *                          compilation time to handle this board.
 *                 ENOENT : this board is not initialized.
 *                 EAGAIN : can't add entry because the LPE is full.
 *                 ERANGE : node out of range.
 ************************************************************
 */

int
put_add_entry(minor, entry)
              int minor;
              lpe_entry_t *entry;
{
  int s;
  lpe_entry_t xentry;
  sm_emu_buf_t sm_buf;
  u_char *sm_buf_p;
  u_long tmp_data;
  int ret;
  size_t firstlen, veryfirstlen;

#ifndef PUT_OPTIMIZE

  if (minor < 0 || minor >= MAX_PCI_INT)
    return ENXIO;
  if (!put_info[minor].lpe)
    return ENOENT;

  if (_PNODE2NODE(entry->routing_part) >= MAX_NODES)
    return ERANGE;

  if (!SM_ISSET(entry->control) && entry->PRSA == NULL)
    panic("put_add_entry: PRSA NULL [%s-%s]\n", debug_stage, debug_stage2);
  if (!SM_ISSET(entry->control) && entry->PLSA == NULL)
    panic("put_add_entry: PLSA NULL [%s-%s]\n", debug_stage, debug_stage2);

#endif /* was PUT_OPTIMIZE */

#ifdef AVOID_LINK_ERROR
  /* Grâce au test qui suit, on est sûr qu'il y a toujours au moins
     deux entrées pour réémettre des pages mal transmises. */
  if (put_get_lpe_free(minor) == 0L)
    return EAGAIN;
#endif

#ifdef PCIDDC2ndRun
  /* NOR => LMI
     NOS => LMP */

#ifndef PUT_OPTIMIZE

  /* IMPORTANT : à cause d'un bug de PCIDDC, on ne doit mettre NOS (i.e.
     demander une IRQ en émission) que pour la derniere page d'un
     message (LMP) */
  if ((entry->control & NOS_MASK) && !(entry->control & LMP_MASK))
    panic("put_add_entry: NOS set but LMP not set\n");

  /* IMPORTANT : on ne doit pas mettre NOR sans LMI (sinon, pour que
     ce soit gerable, il faut que TOUS les messages aient NOR sans LMI
     a partir du moment où au moins un est comme ça : on ne peut
     pas panacher, car le driver en interrupt aurait alors
     l'impossibilité de gérer les IRQ et la LMI... Donc c'est soit
     toujours NOR et LMI ou NOR sans LMI, pour tous les messages.
     Or, NOR sans LMI a peu d'intéret par rapport a NOR avec LMI :
     il y a de toutes facons l'interrupt, on perd seulement le MI. */
  if ((entry->control & NOR_MASK) && !(entry->control & LMI_ENABLE_MASK))
    panic("put_add_entry: NOR set but LMI not set\n");

  /* IMPORTANT : pour l'instant, LMI sans NOR est stupide :
     ca veut dire mettre dans la LMI mais pas prevenir que c'est
     dans la LMI (et ce sera ignoré par le handler d'IRQ). Si
     une fonction get() apparait, ca peut alors avoir son interet. */
  /*
    if ((entry->control & LMI_ENABLE_MASK) && !(entry->control & NOR_MASK))
    panic("put_add_entry: LMI set but NOR not set\n");
  */

#endif /* was PUT_OPTIMIZE */

  /* Short Messages and Misaligned pages are handled correctly in PCIDDC2ndRun */
  return _put_add_entry(minor, entry);
#endif

  /* Short messages have a very particular software bug fix... */
  /* Is this a short message, and do we want to send it over HSL ? */
  if (put_info[minor].remote_hsl[_PNODE2NODE(entry->routing_part)] == TRUE &&
      SM_ISSET(entry->control)) {
    /* Yes, emulate SM over HSL */

    /* This bug fix only allows 128 hosts on the network */
    if (_PNODE2NODE(entry->routing_part) >= 128) return ERANGE;

    s = splhigh();

    /* En principe, le code qui suit ne peut pas etre parcouru,
       car dans le cas du 2eme Run, on fait un return quelques lignes
       auparavant.
       Ce code existe, pour le cas ou on devrait réintroduire
       un correctif au désalignement : le test qui est fait là doit
       absolument être fait après l'entrée de la section
       critique de PUT (splhigh()). */
#ifdef PCIDDC2ndRun
    /* Pas de réentrance */
    if (put_info[minor].stop_put == TRUE) {
      splx(s);
      log(LOG_DEBUG,
	  "put_add_entry(): réentrance bloquée\n");
      return EAGAIN;
    }
#endif

    /* Do we have 8 free LPE entries ? */
    if (put_get_lpe_free(minor) == 0L) {
      splx(s);
      return EAGAIN;
    }

    xentry.page_length  = 4;
    xentry.routing_part = entry->routing_part;
    /* Since we fix a bug in the first run of PCI-DDC, we can assume that the
       LRM is not used. */
    xentry.PRSA         = (caddr_t) vtophys(put_info[minor].hsl_lrm);
    xentry.PLSA         = (caddr_t) vtophys(put_info[minor].hsl_lrm);

    sm_buf.control = entry->control;
    sm_buf.data1   = (u_long) entry->PRSA;
    sm_buf.data2   = (u_long) entry->PLSA;

    sm_buf_p = (u_char *) &sm_buf;

#ifdef DEBUG_HSL
    log(LOG_DEBUG,
	"put_add_entry Short Message: control:0x%x, data1:0x%x, data2:0x%x\n",
	(int) sm_buf.control, (int) sm_buf.data1, (int) sm_buf.data2);
#endif

#define SM_MAKE_CONTROL(seq, data) \
    LMP_MASK | LMI_ENABLE_MASK | SM_EMU_SELECT_MASK | \
      (put_get_node(minor)<<16) | ((seq)<<12) | ((data) & SM_EMU_DATA_MASK)

    /* Send the 12 bytes long sm_buf structure with 8 normal messages */

    /* Send 12 bits of tmp_data */
    tmp_data = ((u_long) sm_buf_p[0]) + ((((u_long) sm_buf_p[8]) & 0x0f)<<8);
    xentry.control = SM_MAKE_CONTROL(0, tmp_data);
    ret = _put_add_entry(minor, &xentry);
    if (ret) log(LOG_ERR, "SM-over-HSL: FATAL ERROR %d\n", ret);

    /* Send 12 bits of tmp_data */
    tmp_data = ((u_long) sm_buf_p[1]) + ((((u_long) sm_buf_p[8]) & 0xf0)<<4);
    xentry.control = SM_MAKE_CONTROL(1, tmp_data);
    ret = _put_add_entry(minor, &xentry);
    if (ret) log(LOG_ERR, "SM-over-HSL: FATAL ERROR %d\n", ret);

    /* Send 12 bits of tmp_data */
    tmp_data = ((u_long) sm_buf_p[2]) + ((((u_long) sm_buf_p[9]) & 0x0f)<<8);
    xentry.control = SM_MAKE_CONTROL(2, tmp_data);
    ret = _put_add_entry(minor, &xentry);
    if (ret) log(LOG_ERR, "SM-over-HSL: FATAL ERROR %d\n", ret);

    /* Send 12 bits of tmp_data */
    tmp_data = ((u_long) sm_buf_p[3]) + ((((u_long) sm_buf_p[9]) & 0xf0)<<4);
    xentry.control = SM_MAKE_CONTROL(3, tmp_data);
    ret = _put_add_entry(minor, &xentry);
    if (ret) log(LOG_ERR, "SM-over-HSL: FATAL ERROR %d\n", ret);

    /* Send 12 bits of tmp_data */
    tmp_data = ((u_long) sm_buf_p[4]) + ((((u_long) sm_buf_p[10]) & 0x0f)<<8);
    xentry.control = SM_MAKE_CONTROL(4, tmp_data);
    ret = _put_add_entry(minor, &xentry);
    if (ret) log(LOG_ERR, "SM-over-HSL: FATAL ERROR %d\n", ret);

    /* Send 12 bits of tmp_data */
    tmp_data = ((u_long) sm_buf_p[5]) + ((((u_long) sm_buf_p[10]) & 0xf0)<<4);
    xentry.control = SM_MAKE_CONTROL(5, tmp_data);
    ret = _put_add_entry(minor, &xentry);
    if (ret) log(LOG_ERR, "SM-over-HSL: FATAL ERROR %d\n", ret);

    /* Send 12 bits of tmp_data */
    tmp_data = ((u_long) sm_buf_p[6]) + ((((u_long) sm_buf_p[11]) & 0x0f)<<8);
    xentry.control = SM_MAKE_CONTROL(6, tmp_data);
    ret = _put_add_entry(minor, &xentry);
    if (ret) log(LOG_ERR, "SM-over-HSL: FATAL ERROR %d\n", ret);

    /* Send 12 bits of tmp_data */
    tmp_data = ((u_long) sm_buf_p[7]) + ((((u_long) sm_buf_p[11]) & 0xf0)<<4);
    xentry.control = SM_MAKE_CONTROL(7, tmp_data);
    ret = _put_add_entry(minor, &xentry);
    if (ret) log(LOG_ERR, "SM-over-HSL: FATAL ERROR %d\n", ret);

    splx(s);
    return 0;

  } else {

    if (put_info[minor].remote_hsl[_PNODE2NODE(entry->routing_part)] == TRUE &&
	(((u_long) entry->PRSA) & 3)) {
      int lpefree;

      /*
       * We must handle misaligned buffers (software fix of hardware bug)
       * This fix only works on non adaptative networks.
       */

      /* log(LOG_DEBUG, "MISALIGNED REMOTE BUFFER\n"); */

      s = splhigh();

      /* At this point, we may send 3 pages. Do we have 3 free LPE entries ? */
      if ((lpefree = put_get_lpe_free(minor)) < 3L /* we could replace 3 by 0
						      since put_get_lpe_free()
						      substracts 7 */) {
	/* log(LOG_ERR, "LPE FREE = %d\n", lpefree); */
	splx(s);
	return EAGAIN;
      }

      /* log(LOG_ERR, "LPE FREE = %d\n", lpefree); */

      if ((((u_long) entry->PRSA) & 3) == (((u_long) entry->PLSA) & 3)) {
	/* Same alignment -> don't copy */

	if (entry->page_length <= 4 - (((u_long) entry->PLSA) & 3)) {
	  /* Send only 1 page */

	  ret = _put_add_entry(minor, entry);
	  if (ret) log(LOG_ERR, "MISASLIGN: FATAL ERROR-1 %d (%d)\n",
		       ret, lpefree);

	  splx(s);
	  return 0;

	} else {
	  /* Send 2 pages */

	  xentry = *entry;
	  xentry.page_length = 4 - (((u_long) entry->PLSA) & 3);
	  xentry.control &= ~(NOS_MASK | NOR_MASK | LMP_MASK);
	  ret = _put_add_entry(minor, &xentry);
	  if (ret) log(LOG_ERR, "MISASLIGN: FATAL ERROR-2 %d (%d)\n",
		       ret, lpefree);

	  /* At this point, the remainding remote buffer is aligned. */

	  xentry = *entry;
	  xentry.page_length -= 4 - (((u_long) entry->PLSA) & 3);
	  xentry.PLSA += 4 - (((u_long) entry->PLSA) & 3);
	  xentry.PRSA += 4 - (((u_long) entry->PLSA) & 3);
	  ret = _put_add_entry(minor, &xentry);
	  if (ret) log(LOG_ERR, "MISASLIGN: FATAL ERROR-3 %d (%d)\n",
		       ret, lpefree);

	  splx(s);
	  return 0;
	}
      } else {
	/*
	 * Not the same alignment -> we will have to copy the sending buffer
	 * in order to have the same alignment that the destination buffer.
	 */

	/* How many bytes must we send to have the remote buffer aligned ? */
	firstlen = 4 - (((u_long) entry->PRSA) & 3);

	/* Does the local buffer cross a long word limit ? */
	if (firstlen + (((u_long) entry->PLSA) & 3) <= 4) {
	  /* No, send 1 page */

	  xentry = *entry;
	  xentry.page_length = firstlen;

	  /* Is this the last message page ? */
	  if (firstlen != entry->page_length)
	    /* No, don't ask for interrupts. */
	    xentry.control &= ~(NOS_MASK | NOR_MASK | LMP_MASK);

	  ret = _put_add_entry(minor, &xentry);
	  if (ret) log(LOG_ERR, "MISASLIGN: FATAL ERROR-4 %d\n", ret);

	  /* At this point, the remainding remote buffer is aligned. */

	  /* Are there some pending data ? */
	  if (firstlen != entry->page_length) {
	    /* Yes, send them */

	    xentry = *entry;
	    xentry.page_length -= firstlen;
	    xentry.PLSA += firstlen;
	    xentry.PRSA += firstlen;
	    ret = _put_add_entry(minor, &xentry);
	    if (ret) log(LOG_ERR, "MISASLIGN: FATAL ERROR-5 %d\n", ret);
	  }

	  splx(s);
	  return 0;

	} else {
	  /* Yes, send 2 pages */

	  veryfirstlen = 4 - (((u_long) entry->PLSA) & 3);

	  xentry = *entry;
	  xentry.page_length = veryfirstlen;
	  xentry.control &= ~(NOS_MASK | NOR_MASK | LMP_MASK);
	  ret = _put_add_entry(minor, &xentry);
	  if (ret) log(LOG_ERR, "MISASLIGN: FATAL ERROR-6 %d\n", ret);

	  xentry = *entry;

	  /* Is this the last message page ? */
	  if (firstlen != entry->page_length)
	    /* No, don't ask for interrupts. */
	    xentry.control &= ~(NOS_MASK | NOR_MASK | LMP_MASK);

	  xentry.page_length = firstlen - veryfirstlen;
	  xentry.PLSA += veryfirstlen;
	  xentry.PRSA += veryfirstlen;
	  ret = _put_add_entry(minor, &xentry);
	  if (ret) log(LOG_ERR, "MISASLIGN: FATAL ERROR-7 %d\n", ret);

	  /* Are there some pending data ? */
	  if (firstlen != entry->page_length) {
	    /* Yes, send them */

	    xentry = *entry;
	    xentry.page_length -= firstlen;
	    xentry.PLSA += firstlen;
	    xentry.PRSA += firstlen;
	    ret = _put_add_entry(minor, &xentry);
	    if (ret) log(LOG_ERR, "MISASLIGN: FATAL ERROR-8 %d\n", ret);

	    splx(s);
	    return 0;
	  }
	  /* Question de Fred : normalement, ici, on doit
	     faire splx(s);return 0; mais en fait on va
	     faire en plus un log FATAL ERROR-9 -> regarder
	     ce probleme */
	}
      }
    } else return _put_add_entry(minor, entry);
  }
  /* NOTREACHED */
  log(LOG_ERR, "MISASLIGN: FATAL ERROR-9\n");
  splx(s);
  return 0;
}


#ifndef AVOID_LINK_ERROR
static
#endif
int
_put_add_entry(minor, entry)
              int minor;
              lpe_entry_t *entry;
{
  int s;
#ifndef PUT_DONT_FLUSH_LPE
  mi_t mi;
#endif

  /*
log(LOG_DEBUG, "PUT_ADD_ENTRY: current=0x%x,new=0x%x,softnew=0x%x\n",
put_info[minor].lpe_current_virt,
put_info[minor].lpe_new_virt,
put_info[minor].lpe_softnew_virt);
  */

#ifdef MPC_STATS
  driver_stats.calls_put_add_entry++;
#endif

  /*VERIFIER QUE page_length est non null avant l'envoi*/

#ifdef DEBUG_HSL
  log(LOG_DEBUG,
      "put_add_entry(%d)\npage length = 0x%x\nrouting part = 0x%x\ncontrol = 0x%x\nPRSA = 0x%x\nPLSA = 0x%x\n",
      minor,
      entry->page_length,
      entry->routing_part,
      (int) entry->control,
      (int) entry->PRSA,
      (int) entry->PLSA);
#endif

#ifndef PUT_OPTIMIZE

  if (minor < 0 || minor >= MAX_PCI_INT)
    return ENXIO;
  if (!put_info[minor].lpe)
    return ENOENT;

  if (_PNODE2NODE(entry->routing_part) >= MAX_NODES)
    return ERANGE;

#endif

  if ((put_info[minor].remote_hsl[_PNODE2NODE(entry->routing_part)] ==
      FALSE)
#ifdef PCIDDC1stRun
      /*
       * Short messages are buggy, send them via Ethernet... This should
       * not happen, since we have made a special fix in put_add_entry(),
       * before calling _put_add_entry().
       */
      || SM_ISSET(entry->control)
#endif
      ) {
    /* Contact remote node via Ethernet */

    s = splhigh();

    /* Can't add entry while the LPE is full */
    if ((put_info[minor].lpe_high + 1) % put_info[minor].nentries == 
	put_info[minor].lpe_low) {
      splx(s);
      return EAGAIN;
    }

    put_info[minor].lpe[put_info[minor].lpe_high] = *entry;
#ifdef _WITH_MONITOR
    put_info[minor].lpe[put_info[minor].lpe_high].routing_part =
      pnode2node[entry->routing_part];
#endif
    put_info[minor].lpe_high++;
    put_info[minor].lpe_high %= put_info[minor].nentries;

    wakeup(&hslgetlpe_ident);

    splx(s);

  } else {
    /* Contact remote node via HSL */

    lpe_entry_t *temp;

    s = splhigh();

#ifdef PCIDDC2ndRun

    /* Pas de réentrance */
    if (put_info[minor].stop_put == TRUE) {
      splx(s);
      log(LOG_DEBUG,
	  "put_add_entry(): réentrance bloquée\n");
      /* c'est le cas où le handler d'interrupt pour données émises
	 serait appelé lors de ce PUT, et où il appelerait à son
	 tour put_add_entry() */
      return EAGAIN;
    }

    /* We need to update the LPE CURRENT pointer to acknowledge the entries
       that have been processed by PCIDDC, otherwise a program that
       would add many LPE entries without asking for interrupts at the
       end of the transmition would let PUT lead to a dead lock :
       LPE full but no interrupt to sweep it...

       On n'a pas besoin de faire un traitement identique pour la LMI :
       si des utilisateurs envoient des entrées de LMI sans NOR,
       ils doivent eux-meme aller regarder dans la LMI (utilité d'une
       fonction get()). De toutes facons, les LMI sans NOR sont actuellement
       consommées lors d'un appel au handler d'interrupt. Ils disparaissent
       donc eventuellement tous seuls, il ne faut donc pas en faire !

       Une consequence de ce traitement particulier est qu'il est
       possible que pendant notre PUT, un handler d'interrupt soit
       appelé par le code qui suit, et que ce handler fasse un autre
       PUT. Hormis le problème de la réentrance, ce scénario
       rentre en conflit avec le scenario traditionnel d'envoi
       d'un message de plusieurs pages :
       - section critique
         - y-a-t-il suffisamment d'entrées libres ?
         - OK, on rajoute les entrées une par une
       - fin de section critique
       En effet, si un PUT inattendu intervient au moment de l'ajout
       d'une entrée au milieu de la section critique,
       on risque d'aboutir à un famine avant la fin de la section
       critique. */

    /* On peut retirer le bloc qui suit pour simplifier la detection
       d'erreurs */

#ifndef PUT_DONT_FLUSH_LPE
    /* We should call put_flush_lpe() instead of inserting this block */
    {
      int i;
      lpe_entry_t *old_current_virt;

      old_current_virt = put_info[minor].lpe_current_virt;

      /* Update our copy of the LPE CURRENT pointer */
      put_info[minor].lpe_current_virt = (lpe_entry_t *)
	(hsl_conf_read(minor, PCIDDC_LPE_CURRENT) -
	 (u_long) put_info[minor].hsl_contig_space_phys +
	 (u_long) put_info[minor].hsl_contig_space);

      if (old_current_virt != put_info[minor].lpe_current_virt)
	/* Signal that some new LPE entries will soon be free */
	wakeup(&hslfreelpe_ident);

      while (old_current_virt != put_info[minor].lpe_current_virt) {

	/* here, we should/could only test NOS_MASK, but a bug in PCIDDC
	   forces us to set LMP when NOS is set, to get an interrupt
	   (and have the counter incremented : to be checked) */
	if ((old_current_virt->control & (NOS_MASK | LMP_MASK)) ==
	    (NOS_MASK | LMP_MASK)) {
	  u_long temp_pciddc_status;

	  temp_pciddc_status = hsl_conf_read(minor,
					     PCIDDC_STATUS);

	  /* Warning : if PAGE_TRANSMITTED is not set, this LPE entry is not completed */
	  /* C'est la condition de sortie de cette boucle */
	  if (!(temp_pciddc_status & PCIDDC_STATUS_PAGE_TRANSMITTED)) {
	    put_info[minor].lpe_current_virt = old_current_virt;
	    break;
	  }

	  /* Acknowledge the entry */
	  /* A method with better performances would be to acknowledge
	     entries 16, 256 or 4096 at a time... */
	  hsl_conf_write(minor,
			 PCIDDC_STATUS,
			 PCIDDC_STATUS_PAGE_TRANSMITTED);

#if defined(LPE_SLOW) || defined(LPE_BURST)
	  /* Recover the real NOS bit value
	     (in order to fix a bug in PCI-DDC) */
	  if (RESERVED_ISSET(old_current_virt->control))
	    old_current_virt->control |=
	      NOS_MASK;
	  else
	    old_current_virt->control &=
	      ~NOS_MASK;
#endif

#ifdef LPE_SLOW
	  /* The DMA is terminated */
	  put_info[(int) arg].lpe_loaded = FALSE;
#endif

#if defined(LPE_SLOW) || defined(LPE_BURST)
	  /* Call the upper layer interrupt handler if the NOS bit was
	     effectively set at the put_add_entry() time */
	  if (RESERVED_ISSET(old_current_virt->control)) {
#endif

	    /* Call the upper layer interrupt handler */
	    mi = MI_PART(old_current_virt->control);
	    for (i = 0; i < MAX_SAP; i++) {
	      void (*fct) __P((int,
			       mi_t,
			       lpe_entry_t));

	      fct = put_info[minor].IT_sent[i];

	      if (fct &&
		  mi >= put_info[minor].mi_space[i].mi_start &&
		  mi < put_info[minor].mi_space[i].mi_start +
		  put_info[minor].mi_space[i].mi_size) {
		put_info[minor].stop_put = TRUE;
#ifdef AVOID_LINK_ERROR
		register_interrupt(interrupt_table_LPE,
				   (old_current_virt->control & 0x00ffff00)>>8,
				   (void (*)()) fct,
				   *old_current_virt,
				   TRUE,
				   (u_long) minor,
				   (u_long) mi);
		fct(minor,
		    mi,
		    *old_current_virt);
#else
		fct(minor,
		    mi,
		    *old_current_virt);
#endif
		put_info[minor].stop_put = FALSE;
		break;
	      }
	    }
#if defined(LPE_SLOW) || defined(LPE_BURST)
	  }
#endif
	}
	
	/* Continue with the next LPE entry to acknowledge */
	if (++old_current_virt ==
	    ((lpe_entry_t *) put_info[minor].hsl_lpe) + HSL_LPE_SIZE)
	  old_current_virt = (lpe_entry_t *) put_info[minor].hsl_lpe;
      }
    }

#endif /* was ifndef PUT_DONT_FLUSH_LPE */

#endif /* was PCIDDC2ndRun */

    /* Let temp be the new softnew */
    temp = put_info[minor].lpe_softnew_virt + 1;
    if (temp == ((lpe_entry_t *) put_info[minor].hsl_lpe) + HSL_LPE_SIZE)
      temp = (lpe_entry_t *) put_info[minor].hsl_lpe;

    /* Can't add entry while the LPE is full */
    if (temp == put_info[minor].lpe_current_virt) {
#ifdef DEBUG_HSL
      log(LOG_DEBUG, "LPE FULL\n");
#endif
      splx(s);
      return EAGAIN;
    }

    /* Add entry into the LPE */
    *put_info[minor].lpe_softnew_virt = *entry;
#ifdef _WITH_MONITOR
    put_info[minor].lpe_softnew_virt->routing_part =
      pnode2node[entry->routing_part];
#endif
#ifdef AVOID_LINK_ERROR
    if (avoid_MI_safe == FALSE) {
      put_info[minor].lpe_softnew_virt->control =
	(put_info[minor].lpe_softnew_virt->control & 0xff0000ff) |
	(avoid_cnt<<8);
      /* L'utilisateur de PUT ne met le flag LMI que pour la dernière
	 page, car parfois PUT est amenée à rajouter (LMP|NOS) pour chaque
	 page intermédiaire. On ne consulte donc pas LMP mais LMI pour
	 détecter la dernière page d'un message. */
      if (LMI_ENABLE_ISSET(put_info[minor].lpe_softnew_virt->control)) {
	if (++avoid_cnt == 65536)
	  avoid_cnt = 0;
      }
    }
#endif

#if defined(PCIDDC1stRun) || defined(LPE_SLOW) || defined(LPE_BURST)
    /* 1st Run or { 2nd Run with recent chipset analog to 440BX }
       (useless with 440LX) */

    /* Due to some bugs in PCI-DDC, we may have to force a NOS in the
       entry, so we backup the old value in the reserved field */
    if (NOS_ISSET(put_info[minor].lpe_softnew_virt->control))
      put_info[minor].lpe_softnew_virt->control |= RESERVED_MASK;
    else
      put_info[minor].lpe_softnew_virt->control &= ~RESERVED_MASK;

#ifdef LPE_BURST
    if (((put_info[minor].lpe_softnew_virt -
	  (lpe_entry_t *) put_info[minor].hsl_lpe) %
	 LPE_BURST_LIMIT) == (LPE_BURST_LIMIT - 1)) {
#endif

      /* Force a NOS */
      put_info[minor].lpe_softnew_virt->control |= NOS_MASK;

      /* Due to a bug in PCI-DDC, we MUST set LMP_MASK for the chip
	 to raise an interrupt when NOS is SET */
      if (NOS_ISSET(put_info[minor].lpe_softnew_virt->control))
	put_info[minor].lpe_softnew_virt->control |= LMP_MASK;

      /* Since we may have added LMP_MASK on an intermediate page
	 of a message, we must disable the LRM (only in case of an
	 adaptative network - we always do that because there is
	 no easy mean to know if the network is adaptative or not) */
      put_info[minor].lpe_softnew_virt->control &= ~LRM_ENABLE_MASK;

      /* We should before copy the LMP_MASK bit in the MI part,
	 in order to allow its reset when received in the LMI... */

#ifdef LPE_BURST
    }
#endif

#else /* PCIDDC 2nd Run with old chipset like 440LX */

    /* The LRM is disabled because it is not handled correctly
       in PCIDDC */
    put_info[minor].lpe_softnew_virt->control &= ~LRM_ENABLE_MASK;

#endif

    /* Increment softnew */
    put_info[minor].lpe_softnew_virt = temp;

#ifdef PCIDDC1stRun

    /* Initiate a DMA only if the previous one is terminated or aborted */
    if (put_info[minor].lpe_loaded == FALSE) {
      u_long plsa;
      size_t len;

      put_info[minor].lpe_loaded = TRUE;

      /*
       * Software fix of hardware bug : local buffer misaligned ?
       * (the remote buffer can not be misaligned at this point)
       */

      plsa = (u_long) put_info[minor].lpe_new_virt->PLSA;
      len  = put_info[minor].lpe_new_virt->page_length;

      if ((plsa & 3) && ((plsa + len - 1) & ~3) != (plsa & ~3)) {
	bstore_phys(put_info[minor].lpe_new_virt->PLSA,
		    misalign_contig_space,
		    put_info[minor].lpe_new_virt->page_length);
	put_info[minor].lpe_new_virt->PLSA = misalign_contig_space_phys;
      }

      /* Increment the LPE_NEW register */
      if (++put_info[minor].lpe_new_virt ==
	  ((lpe_entry_t *) put_info[minor].hsl_lpe) + HSL_LPE_SIZE)
	put_info[minor].lpe_new_virt =
	  (lpe_entry_t *) put_info[minor].hsl_lpe;

      hsl_conf_write(minor,
		     PCIDDC_LPE_NEW,
		     /* We could use PCIDDC_VIRT2PHYS()
			instead of vtophys() */
		     vtophys(put_info[minor].lpe_new_virt));
    }

#else /* PCIDDC 2nd Run */

#if !defined(LPE_SLOW) && !defined(LPE_BURST)
    /* Update the LPE_NEW register */
    put_info[minor].lpe_new_virt = put_info[minor].lpe_softnew_virt;
    hsl_conf_write(minor,
		   PCIDDC_LPE_NEW,
		   /* We could use PCIDDC_VIRT2PHYS()
		      instead of vtophys() */
		   vtophys(put_info[minor].lpe_new_virt));
#endif

#ifdef LPE_SLOW
    if (put_info[minor].lpe_loaded == FALSE &&
	put_info[minor].lpe_new_virt != put_info[minor].lpe_softnew_virt) {
      
      /* Increment the LPE_NEW register */
      if (++put_info[minor].lpe_new_virt ==
	  ((lpe_entry_t *) put_info[minor].hsl_lpe) + HSL_LPE_SIZE)
	put_info[minor].lpe_new_virt =
	  (lpe_entry_t *) put_info[minor].hsl_lpe;

      hsl_conf_write(minor,
		     PCIDDC_LPE_NEW,
		     /* We could use PCIDDC_VIRT2PHYS()
			instead of vtophys() */
		     vtophys(put_info[minor].lpe_new_virt));

      put_info[minor].lpe_loaded = TRUE;
    }
#endif

#ifdef LPE_BURST
    {
      int block_current;
      int block_softnew;
      lpe_entry_t *next_new;

      /* Divide the LPE in several blocks of size LPE_BURST_LIMIT */
      block_current = (put_info[minor].lpe_current_virt -
		       (lpe_entry_t *) put_info[minor].hsl_lpe)
	/ LPE_BURST_LIMIT;
      block_softnew = (put_info[minor].lpe_softnew_virt -
		       (lpe_entry_t *) put_info[minor].hsl_lpe)
	/ LPE_BURST_LIMIT;

      /* If the current pointer is not in the same block than the
	 softnew pointer, let the next new value be the beginning of
	 the block just after the one of the current pointer. */
      if (block_current != block_softnew) {
	next_new = ((lpe_entry_t *) put_info[minor].hsl_lpe) +
	  (block_current + 1) * LPE_BURST_LIMIT;
	if (next_new ==
	    ((lpe_entry_t *) put_info[minor].hsl_lpe) + HSL_LPE_SIZE)
	  next_new =
	    (lpe_entry_t *) put_info[minor].hsl_lpe;
      }
      else
	next_new = put_info[minor].lpe_softnew_virt;

      /* Update the LPE_NEW register */
      if (next_new != put_info[minor].lpe_new_virt) {
	put_info[minor].lpe_new_virt = next_new;

	hsl_conf_write(minor,
		       PCIDDC_LPE_NEW,
		       /* We could use PCIDDC_VIRT2PHYS()
			  instead of vtophys() */
		       vtophys(put_info[minor].lpe_new_virt));
      }
    }
#endif

#endif

    splx(s);
  }

  return 0;
}


/*
 ************************************************************
 * put_flush_lpe() : flush the LPE.
 * should be called at splhigh processor level,
 * and not from an interrupt handler.
 *
 * minor : the board number.
 ************************************************************
 */

void
put_flush_lpe(minor)
             int minor;
{
  mi_t mi;
  int i;
  lpe_entry_t *old_current_virt;

#ifdef PCIDDC1stRun
  panic("put_flush_lpe() not available\n");
#endif

  old_current_virt = put_info[minor].lpe_current_virt;

  /* Update our copy of the LPE CURRENT pointer */
  put_info[minor].lpe_current_virt = (lpe_entry_t *)
    (hsl_conf_read(minor, PCIDDC_LPE_CURRENT) -
     (u_long) put_info[minor].hsl_contig_space_phys +
     (u_long) put_info[minor].hsl_contig_space);

  if (old_current_virt != put_info[minor].lpe_current_virt)
    /* Signal that some new LPE entries will soon be free */
    wakeup(&hslfreelpe_ident);

  while (old_current_virt != put_info[minor].lpe_current_virt) {

    /* here, we should/could only test NOS_MASK, but a bug in PCIDDC
       forces us to set LMP when NOS is set, to get an interrupt
       (and have the counter incremented : to be checked) */
    if ((old_current_virt->control & (NOS_MASK | LMP_MASK)) ==
	(NOS_MASK | LMP_MASK)) {
      u_long temp_pciddc_status;

      temp_pciddc_status = hsl_conf_read(minor,
					 PCIDDC_STATUS);

      /* Warning : if PAGE_TRANSMITTED is not set, this LPE entry is not completed */
      if (!(temp_pciddc_status & PCIDDC_STATUS_PAGE_TRANSMITTED)) {
	put_info[minor].lpe_current_virt = old_current_virt;
	break;
      }

      /* Acknowledge the entry */
      /* A method with better performances would be to acknowledge
	 entries 16, 256 or 4096 at a time... */
      hsl_conf_write(minor,
		     PCIDDC_STATUS,
		     PCIDDC_STATUS_PAGE_TRANSMITTED);

#if defined(LPE_SLOW) || defined(LPE_BURST)
      /* Recover the real NOS bit value
	 (in order to fix a bug in PCI-DDC) */
      if (RESERVED_ISSET(old_current_virt->control))
	old_current_virt->control |=
	  NOS_MASK;
      else
	old_current_virt->control &=
	  ~NOS_MASK;
#endif

#ifdef LPE_SLOW
      /* The DMA is terminated */
      put_info[minor].lpe_loaded = FALSE;
#endif

#if defined(LPE_SLOW) || defined(LPE_BURST)
      /* Call the upper layer interrupt handler if the NOS bit was
	 effectively set at the put_add_entry() time */
      if (RESERVED_ISSET(old_current_virt->control)) {
#endif

	/* Call the upper layer interrupt handler */
	mi = MI_PART(old_current_virt->control);
	for (i = 0; i < MAX_SAP; i++) {
	  void (*fct) __P((int,
			   mi_t,
			   lpe_entry_t));

	  fct = put_info[minor].IT_sent[i];

	  if (fct &&
	      mi >= put_info[minor].mi_space[i].mi_start &&
	      mi < put_info[minor].mi_space[i].mi_start +
	      put_info[minor].mi_space[i].mi_size) {
	    put_info[minor].stop_put = TRUE;
#ifdef AVOID_LINK_ERROR
	    register_interrupt(interrupt_table_LPE,
			       (old_current_virt->control & 0x00ffff00)>>8,
			       (void (*)()) fct,
			       *old_current_virt,
			       TRUE,
			       (u_long) minor,
			       (u_long) mi);
	    fct(minor,
		mi,
		*old_current_virt);
#else
	    fct(minor,
		mi,
		*old_current_virt);
#endif
	    put_info[minor].stop_put = FALSE;
	    break;
	  }
	}
#if defined(LPE_SLOW) || defined(LPE_BURST)
      }
#endif
    }
	
    /* Continue with the next LPE entry to acknowledge */
    if (++old_current_virt ==
	((lpe_entry_t *) put_info[minor].hsl_lpe) + HSL_LPE_SIZE)
      old_current_virt = (lpe_entry_t *) put_info[minor].hsl_lpe;
  }
}


/*
 ************************************************************
 * put_flush_lmi() : flush the LMI.
 * should be called at splhigh processor level,
 * and not from an interrupt handler.
 *
 * minor : the board number.
 ************************************************************
 */

void
put_flush_lmi(minor)
             int minor;
{
  u_long pciddc_status;
  lmi_entry_t *lmi_new_virt;
  int i;
  mi_t mi;

#ifdef PCIDDC1stRun
  panic("put_flush_lmi() not available\n");
#endif

  pciddc_status = hsl_conf_read(minor,
				PCIDDC_STATUS);

  if (pciddc_status & PCIDDC_STATUS_PAGE_RECEIVED) {

    /* Convert LMI_NEW pointer into kernel virtual address space */
    lmi_new_virt = (lmi_entry_t *)
      (hsl_conf_read(minor, PCIDDC_LMI_NEW) -
       (u_long) put_info[minor].hsl_contig_space_phys +
       (u_long) put_info[minor].hsl_contig_space);

    /* Treat and acknowledge each entry in the LMI */
    while (put_info[minor].lmi_current_virt != lmi_new_virt) {

      /* Si on a une entrée de LMI sans NOR, on la signale néanmoins */
      /* if (put_info[minor].lmi_current_virt->control & NOR_MASK) { */
      /* Acknowledge the entry */
      /* A method with better performances would be to acknowledge
	 entries 16, 256 or 4096 at a time... */
      if (put_info[minor].lmi_current_virt->control & NOR_MASK)
	hsl_conf_write(minor,
		       PCIDDC_STATUS,
		       PCIDDC_STATUS_PAGE_RECEIVED);

      /* Call the upper layer interrupt handler */

      mi = MI_PART(put_info[minor].lmi_current_virt->control);
      for (i = 0; i < MAX_SAP; i++) {
	void (*fct) __P((int,
			 mi_t,
			 u_long,
			 u_long));

	fct = put_info[minor].IT_received[i];

	if (fct && mi >= put_info[minor].mi_space[i].mi_start &&
	    mi < put_info[minor].mi_space[i].mi_start +
	    put_info[minor].mi_space[i].mi_size) {
#ifdef AVOID_LINK_ERROR
	  register_interrupt(interrupt_table_LMI,
			     (put_info[minor].lmi_current_virt->control & 0x00ffff00)>>8,
			     (void (*)()) fct,
			     unused_lpe_entry,
			     FALSE,
			     (u_long) minor,
			     (u_long) MI_PART(put_info[minor].lmi_current_virt->control),
			     (u_long) (SM_ISSET(put_info[minor].lmi_current_virt->control) ?
				       (u_int) put_info[minor].lmi_current_virt->data1 : 0),
			     (u_long) (SM_ISSET(put_info[minor].lmi_current_virt->control) ?
				       (u_int) put_info[minor].lmi_current_virt->data2 : 0));
	  fct(minor,
	      MI_PART(put_info[minor].lmi_current_virt->control),
	      SM_ISSET(put_info[minor].lmi_current_virt->control) ?
	      (u_int) put_info[minor].lmi_current_virt->data1 : 0,
	      SM_ISSET(put_info[minor].lmi_current_virt->control) ?
	      (u_int) put_info[minor].lmi_current_virt->data2 : 0);
#else
	  fct(minor,
	      MI_PART(put_info[minor].lmi_current_virt->control),
	      SM_ISSET(put_info[minor].lmi_current_virt->control) ?
	      (u_int) put_info[minor].lmi_current_virt->data1 : 0,
	      SM_ISSET(put_info[minor].lmi_current_virt->control) ?
	      (u_int) put_info[minor].lmi_current_virt->data2 : 0);
#endif
	  break;
	}
      }
      /* } */

      /* Increment lmi_current */
      if (++put_info[minor].lmi_current_virt ==
	  ((lmi_entry_t *) put_info[minor].hsl_lmi) + HSL_LMI_SIZE)
	put_info[minor].lmi_current_virt =
	  (lmi_entry_t *) put_info[minor].hsl_lmi;
    }

    /* Update the new lmi_current value in the chip */
    /* Due to a bug in PCI-DDC,
       LMI_NEW & LMI_CURRENT are reverted as
       they are written... */
    hsl_conf_write(minor,
		   PCIDDC_LMI_NEW /* PCIDDC_LMI_CURRENT */,
		   /* We could use PCIDDC_VIRT2PHYS()
		      instead of vtophys() */
		   vtophys(put_info[minor].lmi_current_virt));

    /*
     * Here, we should examine the PAGE RECEIVED bit in the
     * PCI-DDC status register and, if set,
     * try again the treatment of messages received,
     * in order to deal with messages arrived during the first
     * treatment. This may increase performances...
     */
    }
}


/*
 ************************************************************
 * put_get_entry(minor, entry) : read the oldest entry in the LPE
 *  but don't flush it.
 * minor : board number.
 * entry : buffer for the entry.
 *
 * return values : 0      : success.
 *                 ENXIO  : not enough structures allocated at
 *                          compilation time to handle this board.
 *                 ENOENT : this board is not initialized.
 *                 EAGAIN : LPE empty.
 ************************************************************
 */

int
put_get_entry(minor, entry)
              int minor;
              lpe_entry_t *entry;
{
  int s;

  TRY("put_get_entry")

#ifdef DEBUG_HSL
  log(LOG_DEBUG,
      "put_get_entry(%d, %p)\n",
      minor,
      entry);
#endif

  if (minor < 0 || minor >= MAX_PCI_INT)
    return ENXIO;
  if (!put_info[minor].lpe)
    return ENOENT;

  s = splhigh();

  if (put_info[minor].lpe_high == put_info[minor].lpe_low) {
    splx(s);
    return EAGAIN;
  }
  *entry = put_info[minor].lpe[put_info[minor].lpe_low];

  splx(s);

  return 0;
}


/*
 ************************************************************
 * put_flush_entry(minor) : flush the oldest entry because
 * corresponding data are sent.  This can lead to the simulation of an
 * interrupt if this was the last page of a message.
 * minor : board number.
 *
 * return values : 0      : success.
 *                 ENXIO  : not enough structures allocated at
 *                          compilation time to handle this board.
 *                 ENOENT : this board is not initialized.
 *                 EFAULT : no entry to flush.
 ************************************************************
 */

int
put_flush_entry(minor)
                int minor;
{
  int i, s;
  mi_t mi;
  lpe_entry_t entry;
  void (*fct) __P((int,
		   mi_t,
		   lpe_entry_t));

  TRY("put_flush_entry")

#ifdef DEBUG_HSL
  log(LOG_DEBUG,
      "put_flush_entry(%d)\n",
      minor);
#endif

  if (minor < 0 || minor >= MAX_PCI_INT)
    return ENXIO;
  if (!put_info[minor].lpe)
    return ENOENT;

  s = splhigh();

  if (put_info[minor].lpe_high == put_info[minor].lpe_low) {
    splx(s);
    return EFAULT;
  }

  entry = put_info[minor].lpe[put_info[minor].lpe_low];

  if (++put_info[minor].lpe_low == put_info[minor].nentries)
    put_info[minor].lpe_low = 0;

  if (NOS_ISSET(entry.control))
    for (i = 0;
	 i < MAX_SAP;
	 i++) {
      fct = put_info[minor].IT_sent[i];
      mi = MI_PART(entry.control);

#ifdef DEBUG_HSL
      log(LOG_DEBUG, "put_flush_entry() : fct = %p, mi = %d\n", fct, (int) mi);
#endif

      if (fct &&
          mi >= put_info[minor].mi_space[i].mi_start &&
          mi < put_info[minor].mi_space[i].mi_start +
          put_info[minor].mi_space[i].mi_size) {
#ifdef AVOID_LINK_ERROR
	register_interrupt(interrupt_table_LPE,
			   (entry.control & 0x00ffff00)>>8,
			   (void (*)()) fct,
			   entry,
			   TRUE,
			   (u_long) minor,
			   (u_long) mi);
	fct(minor,
	    mi,
	    entry);
#else
	fct(minor,
	    mi,
	    entry);
#endif
	break;
      }
    }

  /* Signal that a new LPE entry is free */
  wakeup(&hslfreelpe_ident);
  splx(s);

  return 0;
}


/*
 ************************************************************
 * put_simulate_interrupt(minor, lpe) : test the flags in the
 * LPE entry and simulate interrupts if needed.
 * minor : board number.
 * lpe   : LPE entry to process.
 *
 * return values : 0      : success.
 *                 ENXIO  : not enough structures allocated at
 *                          compilation time to handle this board.
 *                 ENOENT : this board is not initialized.
 ************************************************************
 */ 

int
put_simulate_interrupt(minor, lpe)
                       int minor;
                       lpe_entry_t lpe;
{
  int i, s;
  mi_t mi;
  void (*fct) __P((int,
		   mi_t,
		   u_long,
		   u_long));

  TRY("put_simulate_interrupt")

#ifdef DEBUG_HSL
    {
      mi_t mi_debug;

      mi_debug = MI_PART(lpe.control);
      log(LOG_DEBUG,
	  "put_simulate_interrupt(%d, %d)\n",
	  minor,
	  (int) mi_debug);
    }
#endif

  if (minor < 0 || minor >= MAX_PCI_INT)
    return ENXIO;
  if (!put_info[minor].lpe)
    return ENOENT;

  if (NOR_ISSET(lpe.control))
    for (i = 0; i < MAX_SAP; i++) {
      fct = put_info[minor].IT_received[i];
      mi = MI_PART(lpe.control);
      if (fct && mi >= put_info[minor].mi_space[i].mi_start &&
	  mi < put_info[minor].mi_space[i].mi_start +
	  put_info[minor].mi_space[i].mi_size) {
	s = splhigh(); /* Should be splnet() */
#ifdef AVOID_LINK_ERROR
	register_interrupt(interrupt_table_LMI,
			   (lpe.control & 0x00ffff00)>>8,
			   (void (*)()) fct,
			   unused_lpe_entry,
			   FALSE,
			   (u_long) minor,
			   (u_long) MI_PART(lpe.control),
			   (u_long) (SM_ISSET(lpe.control) ? (u_int) lpe.PRSA : 0),
			   (u_long) (SM_ISSET(lpe.control) ? (u_int) lpe.PLSA : 0));
	fct(minor,
	    MI_PART(lpe.control),
	    SM_ISSET(lpe.control) ? (u_int) lpe.PRSA : 0,
	    SM_ISSET(lpe.control) ? (u_int) lpe.PLSA : 0);
#else
	fct(minor,
	    MI_PART(lpe.control),
	    SM_ISSET(lpe.control) ? (u_int) lpe.PRSA : 0,
	    SM_ISSET(lpe.control) ? (u_int) lpe.PLSA : 0);
#endif
	splx(s);
	break;
      }
    }

  return 0;
}


#ifdef _WITH_MONITOR
/*
 ************************************************************
 * put_get_dtp(table) : build the pnode2node table.
 * table : a node2pnode table.
 *
 * return value : 0      : success.
 *                EFAULT : bad address.
 ************************************************************
 */ 

int
put_get_dtp(table)
            pnode_t *table;
{
  int i;
  int res;

  pnode_t table_in[MAX_NODES];

  res = copyin(table, table_in, sizeof table_in);
  if (res < 0)
    return EFAULT;

  for (i = 0; i < MAX_NODES; i++)
    pnode2node[i] = (node_t) -1; 

  for (i = 0; i < MAX_NODES; i++)
    if (table_in[i] != (pnode_t) -1)
      pnode2node[table_in[i]] = i;

  return 0;
}
#endif


#ifdef _WITH_DDSCP
/*
 ************************************************************
 * put_flush_LRM(mi) : flush a LRM entry.
 * mi : index of the entry in the LRM.
 *
 * return value : 0      : success.
 *                ERANGE : mi out of range.
 ************************************************************
 */ 

int
put_flush_LRM(minor, mi)
              int minor;
              mi_t mi;
{
  if (mi < HSL_LRM_SIZE) {
  (*put_info[minor].hsl_lrm)[mi].rpn =
    (*put_info[minor].hsl_lrm)[mi].epn = 0;
  return 0;
  } else
    return ERANGE;
}
#endif


/*
 ************************************************************
 * put_interrupt_handler(arg) : PCI interrupt handler.
 * arg : board number.
 *
 * NOTE THAT WE DON'T RUN AT splhigh PROCESSOR LEVEL HERE.
 ************************************************************
 */ 

#ifdef PCIDDC1stRun

static void
put_interrupt_handler(arg)
                      void *arg;
{
  u_long pciddc_status, pci_status;
  u_long val;
#ifndef NO_CONFIG_READ
  lmi_entry_t *lmi_new_virt;
#endif
  mi_t mi;
  boolean_t should_halt = FALSE;
  boolean_t set_bus_master_up = FALSE;
  int i;
  /* Definition to fix a hardware bug */
  boolean_t call_upper_layer;

  /*
   * Job to do in case of interrupt
   *
   *            /\  __
   *        PR /  \ PR
   *          /    \
   *     MessRec.  /\  __
   *            PT/  \ PT
   *             /    \
   *       PageTrans. HardwareErr.
   */
#ifdef NO_CONFIG_READ
  enum { NOTHING, MESSAGE_RECEIVED, PAGE_TRANSMITTED, HARDWARE_ERROR }
  job = NOTHING;
#endif

#define DEBUG_PCIDDC_RUN_2_BAD_INT

#ifdef DEBUG_PCIDDC_RUN_2_BAD_INT
  {
    u_long pciddcstatus = hsl_conf_read((int) arg,
					PCIDDC_STATUS);
    u_long pcicommandstatus = hsl_conf_read((int) arg,
					    PCI_COMMAND_STATUS_REG);

    if (!(pcicommandstatus & PCI_STATUS_PARITY_ERROR) &&
	!(pciddcstatus & (PCIDDC_STATUS_PAGE_TRANSMITTED |
			  PCIDDC_STATUS_PAGE_RECEIVED |
			  PCIDDC_STATUS_PAGE_TRANSMITTED_4096 |
			  PCIDDC_STATUS_PAGE_TRANSMITTED_256 |
			  PCIDDC_STATUS_PAGE_TRANSMITTED_16 |
			  PCIDDC_STATUS_PAGE_RECEIVED_4096 |
			  PCIDDC_STATUS_PAGE_RECEIVED_256 |
			  PCIDDC_STATUS_PAGE_RECEIVED_16 |
			  PCIDDC_STATUS_EEP_ERROR |
			  PCIDDC_STATUS_CRC_HEADER_ERROR |
			  PCIDDC_STATUS_CRC_DATA_ERROR |
			  PCIDDC_STATUS_EP_ERROR |
			  PCIDDC_STATUS_TIMEOUT_ERROR |
			  PCIDDC_STATUS_R3_STATUS_ERROR |
			  PCIDDC_STATUS_PAGE_TRANSMITTED_OF_ERROR |
			  PCIDDC_STATUS_PAGE_RECEIVED_OF_ERROR |
			  PCIDDC_STATUS_SENT_PACKET_OF_ERROR |
			  PCIDDC_STATUS_RECEIVED_PACKET_OF_ERROR |
			  PCIDDC_STATUS_ILLEGAL_LRM_ACCESS_ERROR |
			  PCIDDC_STATUS_LMI_OF_ERROR))) {
      driver_stats.unattended_interrupts++;
      return;
    }
  }
#endif


#ifdef DEBUG_HSL
  log(LOG_DEBUG,
      "put_interrupt_handler(minor=%d)\n", (int) arg);
#endif

#ifdef MPC_STATS
  driver_stats.calls_put_interrupt++;
#endif

  /*
   * Evaluation of pci_conf_write (pci_conf_read should take longer time...) :
   * the configuration write costs ~1.7µs. If we replace it by the following method,
   * we save 0.4µs. The cost of the io write (outl) seems to be about 0.3 or 0.4µs.
   * 3 outl are needed to perform 1 conf_write.
   * Thoses results were computed on a P100 with an HP Logic Analyser.
   * For further informations, please feel free to contact F. Wasjbürt and A. Fenyö.
   *

  BENCH for the HP Logic Analyser :
  {
    struct pcibus *p;
    int s;

    s = splhigh();
    p = getpcibus();
    p->pb_write(put_info[(int) arg].hsl_tag, 0xfc, 0x1234567);
    p->pb_write(put_info[(int) arg].hsl_tag, 0xfc, 0x1234567);
    p->pb_write(put_info[(int) arg].hsl_tag, 0xfc, 0x1234567);
    p->pb_write(put_info[(int) arg].hsl_tag, 0xfc, 0x1234567);
    p->pb_write(put_info[(int) arg].hsl_tag, 0xfc, 0x1234567);
    splx(s);
  }
  */

#ifdef NO_CONFIG_READ
  if (LMI_ENABLE_ISSET(put_info[(int) arg].lmi_current_virt->control))

    /*
     * Since LMI_ENABLE is always set in a LMI entry, testing the
     * LMI_CURRENT content against 0 indicates that a message has been
     * fully received. There may be some other interrupt sources, this
     * interrupt handler will be called again to handle them.
     */

    job = MESSAGE_RECEIVED;
  else
    switch (put_info[(int) arg].lpe_loaded) {
    case TRUE:
      job = PAGE_TRANSMITTED;

      /*
       * Here, we are not absolutely sure that a page has been
       * actually transmitted. The problem is that we MUST avoid
       * reading any register in the configuration space while a page
       * is being received. But we cannot know that a page has really
       * been transmitted without reading in the configuration space
       * of PCI-DDC. Thus, we assume that a page has really been
       * transmitted, and so we clear (further in the handler) the
       * PAGE_TRANSMITTED interrupt source and set to FALSE the
       * lpe_loaded variable to assume that after the end of this
       * handler, no other page is in process of being sent. If a page
       * was not really transmitted, we know that another source of
       * interrupt was set. This means that we will soon return in the
       * interrupt handler, with a lpe_loaded variable set to
       * FALSE. At this time, we will be sure that neither a message
       * received nor a page transmitted event occured, then it will
       * be sure that a hardware error occured. NOTE that we will then
       * be obliged to halt the chip, because we may have acknowledged
       * a page transmitted during this process, furthermore it is
       * possible that our configuration read have corrupted some data
       * in a new received message.
       *
       * Please feel free to contact Franck Wasjbürt, Cyril Spasevski,
       * Jeanlou Desbarbieux or Alexandre Fenyö for further
       * informations about those methods used in this code to bypass
       * hardware errors.
       */
      break;

    case FALSE:
      job = HARDWARE_ERROR;
      break;
    }
#endif

  /* Check the PCI-DDC status register */

#ifdef NO_CONFIG_READ
  if (job == HARDWARE_ERROR) {
    /*
     * Since we will read on the configuration space the next line,
     * received data may be corrupted, we MUST halt PCI-DDC at the
     * end of the handler.
     * NOTE that the LMI overflow interrupt source is not handled
     * in the NO_CONFIG_READ version of the code. We expect the
     * LMI to be never full... :-)
     */
    should_halt = TRUE;
#endif

    pciddc_status = hsl_conf_read((int) arg,
				  PCIDDC_STATUS);

    log(LOG_ERR,
	"                                status: 0x%x\n", (int) pciddc_status);

    if (pciddc_status & PCIDDC_STATUS_EEP_ERROR) {
      log(LOG_ERR,
	  "hardware error: Exceptional End of Packet Error\n");
      should_halt = TRUE;
     }

    if (pciddc_status & PCIDDC_STATUS_CRC_HEADER_ERROR) {
      log(LOG_ERR,
	  "hardware error: CRC Header Error\n");
      should_halt = TRUE;
    }

    if (pciddc_status & PCIDDC_STATUS_CRC_DATA_ERROR) {
      log(LOG_ERR,
	  "hardware error: CRC Data Error\n");
      should_halt = TRUE;
    }

    if (pciddc_status & PCIDDC_STATUS_EP_ERROR) {
      log(LOG_ERR,
	  "hardware error: End of Packet Error\n");
      should_halt = TRUE;
    }

    if (pciddc_status & PCIDDC_STATUS_TIMEOUT_ERROR) {
      log(LOG_ERR,
	  "hardware error: Timeout Error\n");
      should_halt = TRUE;
    }

    if (pciddc_status & PCIDDC_STATUS_R3_STATUS_ERROR) {
      log(LOG_ERR,
	  "hardware error: R3 Status Error\n");
      should_halt = TRUE;
    }

    if (pciddc_status & PCIDDC_STATUS_PAGE_TRANSMITTED_OF_ERROR) {
      log(LOG_ERR,
	  "hardware error: Page Transmitted Overflow Error\n");
      /* This is not really an error, but we want to know
	 when this condition appears */
      should_halt = TRUE;
    }

    if (pciddc_status & PCIDDC_STATUS_PAGE_RECEIVED_OF_ERROR) {
      log(LOG_ERR,
	  "hardware error: Page Received Overflow Error\n");
      /* This is not really an error, but we want to know
	 when this condition appears */
      should_halt = TRUE;
    }

    if (pciddc_status & PCIDDC_STATUS_SENT_PACKET_OF_ERROR) {
      log(LOG_ERR,
	  "hardware error: Sent Packet Overflow Error\n");
      /* This is not always an error, but we want to know
	 when this condition appears */
      should_halt = TRUE;
    }

    if (pciddc_status & PCIDDC_STATUS_RECEIVED_PACKET_OF_ERROR) {
      log(LOG_ERR,
	  "hardware error: Received Packet Overflow Error\n");
      /* This is not always an error, but we want to know
	 when this condition appears */
      should_halt = TRUE;
    }

    if (pciddc_status & PCIDDC_STATUS_ILLEGAL_LRM_ACCESS_ERROR) {
      log(LOG_ERR,
	  "hardware error: Illegal LRM Access Error\n");
      should_halt = TRUE;
    }

    /* Check the PCI-DDC command & status register */

    pci_status = hsl_conf_read((int) arg,
			       PCI_COMMAND_STATUS_REG);

    /* Due to a bug of PCIDDC, the PCI_STATUS_PARITY_DETECT bit may
       be set but there is no interrupt generated for this condition.
       Thus, we decide to forget that the bit was set... */
    pci_status &= ~PCI_STATUS_PARITY_DETECT;

  /*  log(LOG_DEBUG,
      "command & status: 0x%x\n", status);*/
    log(LOG_ERR,
	"                                                          C&S: 0x%x\n", (int) pci_status);

    if (!(pci_status & PCI_COMMAND_MASTER_ENABLE))
      log(LOG_ERR,
	  "hardware warning: BUS MASTER down\n");

    if (pci_status & PCI_STATUS_PARITY_DETECT) {
      log(LOG_ERR,
	  "hardware error: Detected Parity Error\n");
      should_halt = TRUE;
    }

    if (pci_status & PCI_STATUS_SPECIAL_ERROR) {
      log(LOG_ERR,
	  "hardware error: Signaled System Error\n");
      should_halt = TRUE;
    }

    if (pci_status & PCI_STATUS_MASTER_ABORT) {
      log(LOG_ERR,
	  "hardware error: Received Master Abort Error\n");
      should_halt = TRUE;
    }

    if (pci_status & PCI_STATUS_MASTER_TARGET_ABORT) {
      log(LOG_ERR,
	  "hardware error: Received Target Abort Error\n");
      should_halt = TRUE;
    }

    if (pci_status & PCI_STATUS_TARGET_TARGET_ABORT) {
      log(LOG_ERR,
	  "hardware error: Signaled Target Abort Error\n");
      should_halt = TRUE;
    }

    if (pci_status & PCI_STATUS_PARITY_ERROR) {
      log(LOG_ERR,
	  "hardware error: Data Parity Error Detected\n");
      should_halt = TRUE;
    }

    if (pciddc_status & PCIDDC_STATUS_LMI_OF_ERROR) {
      /* We must reactivate the Bus Master */
      /* pour tester le traitement de LMI_OVERFLOW, on peut simplement
	 ne pas avancer le pointeur LMI_CURRENT a chaque interruption
	 mais uniquement quand LMI_OVERFLOW se produit ! */
      set_bus_master_up = TRUE;

      /*
       * The LMI overflow is not handled correctly by the
       * NO_CONFIG_READ code : if it happens, the chip is halted.
       * Thus, the LMI must be sized big enough and the upper layer
       * interrupt handlers written fast enough not to produce LMI
       * overflows...
       */

#ifdef DEBUG_HSL
      log(LOG_DEBUG,
	  "hardware warning: LMI Overflow Error\n");
#endif
    }

#ifdef NO_CONFIG_READ  
  }
#endif

  if (should_halt == TRUE)
    goto halt;

#ifdef NO_CONFIG_READ
  if (job == MESSAGE_RECEIVED) {
#else
  if (pciddc_status & PCIDDC_STATUS_PAGE_RECEIVED) {
#endif
    /* Treatment of received messages */

#ifndef NO_CONFIG_READ
    /* Convert LMI_NEW pointer into kernel virtual address space */
    lmi_new_virt = (lmi_entry_t *)
      (hsl_conf_read((int) arg, PCIDDC_LMI_NEW) -
       (u_long) put_info[(int) arg].hsl_contig_space_phys +
       (u_long) put_info[(int) arg].hsl_contig_space);
#endif

    /* Treat and acknowledge each entry in the LMI */
#ifndef NO_CONFIG_READ
    while (put_info[(int) arg].lmi_current_virt != lmi_new_virt) {
#else
    while (LMI_ENABLE_ISSET(put_info[(int) arg].lmi_current_virt->control)) {
#endif

      /* Acknowledge the entry */
      /* A method with better performances would be to acknowledge
         entries 16, 256 or 4096 at a time... */
      hsl_conf_write((int) arg,
		     PCIDDC_STATUS,
		     PCIDDC_STATUS_PAGE_RECEIVED);

      /* Recover the real NOS bit value (bug fix of PCI-DDC) */
      /* This bug fix doesn't work, because of another bug in PCI-DDC :
	 the RESERVED bit is not transmitted into the LMI */
      if (RESERVED_ISSET(put_info[(int) arg].lmi_current_virt->control))
	put_info[(int) arg].lmi_current_virt->control |=
	  NOS_MASK;
      else
	put_info[(int) arg].lmi_current_virt->control &=
	  ~NOS_MASK;

      /* Reset the RESERVED bit */
      put_info[(int) arg].lmi_current_virt->control &=
	~RESERVED_MASK;

      /************************************************************/

      call_upper_layer = TRUE;

      /* #ifdef PCIDDC1stRun */
      /* Is this part of a Short Message ? */
      if (SM_EMU_SELECT_ISSET(put_info[(int) arg].
			      lmi_current_virt->control)) {
	/* Yes, this is part of a Short Message */

	node_t       remote_host; /* This is actually a pnode */
	lmi_entry_t *lmi_entry_p;
	int          seq;
	u_long       data;
	u_char       data_octet;
	u_char       data_quartet;
	u_char      *emu_buf_tab_p;

	lmi_entry_p   = put_info[(int) arg].lmi_current_virt;
	remote_host   = (lmi_entry_p->control & SM_EMU_HOST_MASK)>>16;
	seq           = (lmi_entry_p->control & SM_EMU_OFFSET_MASK)>>12;
	data          =  lmi_entry_p->control & SM_EMU_DATA_MASK;
	data_octet    = data & 0xff;
	data_quartet  = data>>8;
	emu_buf_tab_p = (u_char *) (emu_buf_tab + remote_host);

	switch (seq) {
	case 0:
	  emu_buf_tab_p[0]  = data_octet;
	  emu_buf_tab_p[8] |= data_quartet;
	  break;

	case 1:
	  emu_buf_tab_p[1]  = data_octet;
	  emu_buf_tab_p[8] |= data_quartet<<4;
	  break;

	case 2:
	  emu_buf_tab_p[2]  = data_octet;
	  emu_buf_tab_p[9] |= data_quartet;
	  break;

	case 3:
	  emu_buf_tab_p[3]  = data_octet;
	  emu_buf_tab_p[9] |= data_quartet<<4;
	  break;

	case 4:
	  emu_buf_tab_p[4]   = data_octet;
	  emu_buf_tab_p[10] |= data_quartet;
	  break;

	case 5:
	  emu_buf_tab_p[5]   = data_octet;
	  emu_buf_tab_p[10] |= data_quartet<<4;
	  break;

	case 6:
	  emu_buf_tab_p[6]   = data_octet;
	  emu_buf_tab_p[11] |= data_quartet;
	  break;

	case 7:
	  emu_buf_tab_p[7]   = data_octet;
	  emu_buf_tab_p[11] |= data_quartet<<4;
	  break;
	}

	/* Assume that a new part of a MI has been received */
	emu_buf_tab_cpt[remote_host]++;	

	/* Is this the last part ? */
	if (emu_buf_tab_cpt[remote_host] == 8) {
	  /* Yes, fill the entry in */

          lmi_entry_p->packet_number = 1;
	  /* Leave lmi_entry_p->{r3_status, reserved} unchanged */
	  lmi_entry_p->control = emu_buf_tab[remote_host].control;
	  lmi_entry_p->data1   = emu_buf_tab[remote_host].data1;
	  lmi_entry_p->data2   = emu_buf_tab[remote_host].data2;

#ifdef DEBUG_HSL
	  log(LOG_DEBUG,
	      "INTERRUPT Short Message: control:0x%x, data1:0x%x, data2:0x%x\n",
	      (int) lmi_entry_p->control,
	      (int) lmi_entry_p->data1,
	      (int) lmi_entry_p->data2);
#endif

	  emu_buf_tab_cpt[remote_host] = 0;
	  bzero(emu_buf_tab + remote_host, sizeof(sm_emu_buf_t));

	} else call_upper_layer = FALSE;
      }
      /*#endif*/

      /************************************************************/

      /* Call the upper layer interrupt handler */

      if (call_upper_layer == TRUE) {
	mi = MI_PART(put_info[(int) arg].lmi_current_virt->control);

	for (i = 0; i < MAX_SAP; i++) {
	  void (*fct) __P((int,
			   mi_t,
			   u_long,
			   u_long));

	  fct = put_info[(int) arg].IT_received[i];

	  if (fct && mi >= put_info[(int) arg].mi_space[i].mi_start &&
	      mi < put_info[(int) arg].mi_space[i].mi_start +
	      put_info[(int) arg].mi_space[i].mi_size) {
#ifdef AVOID_LINK_ERROR
	    register_interrupt(interrupt_table_LMI,
			       (put_info[(int) arg].lmi_current_virt->control & 0x00ffff00)>>8,
			       (void (*)()) fct,
			       unused_lpe_entry,
			       FALSE,
			       (u_long) arg,
			       (u_long) MI_PART(put_info[(int) arg].lmi_current_virt->control),
			       (u_long) (SM_ISSET(put_info[(int) arg].lmi_current_virt->control) ?
					 (u_int) put_info[(int) arg].lmi_current_virt->data1 : 0),
			       (u_long) (SM_ISSET(put_info[(int) arg].lmi_current_virt->control) ?
					 (u_int) put_info[(int) arg].lmi_current_virt->data2 : 0));
	    fct((int) arg,
		MI_PART(put_info[(int) arg].lmi_current_virt->control),
		SM_ISSET(put_info[(int) arg].lmi_current_virt->control) ?
		(u_int) put_info[(int) arg].lmi_current_virt->data1 : 0,
		SM_ISSET(put_info[(int) arg].lmi_current_virt->control) ?
		(u_int) put_info[(int) arg].lmi_current_virt->data2 : 0);
#else
	    fct((int) arg,
		MI_PART(put_info[(int) arg].lmi_current_virt->control),
		SM_ISSET(put_info[(int) arg].lmi_current_virt->control) ?
		(u_int) put_info[(int) arg].lmi_current_virt->data1 : 0,
		SM_ISSET(put_info[(int) arg].lmi_current_virt->control) ?
		(u_int) put_info[(int) arg].lmi_current_virt->data2 : 0);
#endif
	    break;
	  }
	}
      }

#ifdef NO_CONFIG_READ
      /* Reset the LMI_ENABLE bit to assume that this entry is free */
      put_info[(int) arg].lmi_current_virt->control &=
	~LMI_ENABLE_MASK;
#endif

      /* Increment lmi_current */
      if (++put_info[(int) arg].lmi_current_virt ==
	  ((lmi_entry_t *) put_info[(int) arg].hsl_lmi) + HSL_LMI_SIZE)
	put_info[(int) arg].lmi_current_virt =
	  (lmi_entry_t *) put_info[(int) arg].hsl_lmi;
    }

    /* Update the new lmi_current value in the chip */
    /* Due to a bug in PCI-DDC,
       LMI_NEW & LMI_CURRENT are reverted as
       they are written... */
    hsl_conf_write((int) arg,
		   PCIDDC_LMI_NEW /* PCIDDC_LMI_CURRENT */,
		   /* We could use PCIDDC_VIRT2PHYS()
		      instead of vtophys() */
		   vtophys(put_info[(int) arg].lmi_current_virt));

    /*
     * Here, we should examine the PAGE RECEIVED bit in the
     * PCI-DDC status register and, if set,
     * try again the treatment of messages received,
     * in order to deal with messages arrived during the first
     * treatment. This may increase performances...
     */
  }

#ifdef NO_CONFIG_READ
  if (job == PAGE_TRANSMITTED) {
#else
  if (pciddc_status & PCIDDC_STATUS_PAGE_TRANSMITTED) {
#endif

    /* Treatment of transmitted messages */

    /* Acknowledge the entry */
    /* A method with better performances would be to acknowledge
       entries 16, 256 or 4096 at a time... */
    hsl_conf_write((int) arg,
		   PCIDDC_STATUS,
		   PCIDDC_STATUS_PAGE_TRANSMITTED);

    /* Recover the real NOS bit value
       (in order to fix a bug in PCI-DDC) */
    if (RESERVED_ISSET(put_info[(int) arg].lpe_current_virt->control))
      put_info[(int) arg].lpe_current_virt->control |=
	NOS_MASK;
    else
      put_info[(int) arg].lpe_current_virt->control &=
	~NOS_MASK;

    /* The DMA is terminated */
    put_info[(int) arg].lpe_loaded = FALSE;

    /* Call the upper layer interrupt handler if the NOS bit was
       effectively set at the put_add_entry() time */
    if (RESERVED_ISSET(put_info[(int) arg].lpe_current_virt->control)) {
      mi = MI_PART(put_info[(int) arg].lpe_current_virt->control);

      for (i = 0; i < MAX_SAP; i++) {
	void (*fct) __P((int,
			 mi_t,
			 lpe_entry_t));

	fct = put_info[(int) arg].IT_sent[i];

	if (fct &&
	    mi >= put_info[(int) arg].mi_space[i].mi_start &&
	    mi < put_info[(int) arg].mi_space[i].mi_start +
	    put_info[(int) arg].mi_space[i].mi_size) {
#ifdef AVOID_LINK_ERROR
	  register_interrupt(interrupt_table_LPE,
			     (put_info[(int) arg].lpe_current_virt->control & 0x00ffff00)>>8,
			     (void (*)()) fct,
			     *put_info[(int) arg].lpe_current_virt,
			     TRUE,
			     (u_long) arg,
			     (u_long) mi);
	  fct((int) arg,
	      mi,
	      *put_info[(int) arg].lpe_current_virt);
#else
	  fct((int) arg,
	      mi,
	      *put_info[(int) arg].lpe_current_virt);
#endif
	  break;
	}
      }
    }

    /* Increment lpe_current */
    if (++put_info[(int) arg].lpe_current_virt ==
	((lpe_entry_t *) put_info[(int) arg].hsl_lpe) + HSL_LPE_SIZE)
      put_info[(int) arg].lpe_current_virt =
	(lpe_entry_t *) put_info[(int) arg].hsl_lpe;

    /* If lpe_current differs from lpe_softnew, initiate a DMA */
    if (put_info[(int) arg].lpe_current_virt !=
	put_info[(int) arg].lpe_softnew_virt) {
      u_long plsa;
      size_t len;

      /* Forbid put_add_entry() to ask for another DMA until the
	 end of this one */
      put_info[(int) arg].lpe_loaded = TRUE;

      /*
       * Software fix of hardware bug : local buffer misaligned ?
       * (the remote buffer can not be misaligned at this point)
       */

      plsa = (u_long) put_info[(int) arg].lpe_new_virt->PLSA;
      len  = put_info[(int) arg].lpe_new_virt->page_length;

      if ((plsa & 3) && ((plsa + len - 1) & ~3) != (plsa & ~3)) {
	bstore_phys(put_info[(int) arg].lpe_new_virt->PLSA,
		    misalign_contig_space,
		    put_info[(int) arg].lpe_new_virt->page_length);
	put_info[(int) arg].lpe_new_virt->PLSA = misalign_contig_space_phys;
      }

      /* Increment the LPE_NEW register */
      if (++put_info[(int) arg].lpe_new_virt ==
	  ((lpe_entry_t *) put_info[(int) arg].hsl_lpe) + HSL_LPE_SIZE)
	put_info[(int) arg].lpe_new_virt =
	  (lpe_entry_t *) put_info[(int) arg].hsl_lpe;

      hsl_conf_write((int) arg,
		     PCIDDC_LPE_NEW,
		     /* We could use PCIDDC_VIRT2PHYS()
			instead of vtophys() */
		     vtophys(put_info[(int) arg].lpe_new_virt));
    }

    /* Signal that a new LPE entry is free */
    wakeup(&hslfreelpe_ident);
  }

  /* In case of LMI overflow, we must reactivate the Bus Master */
  if (set_bus_master_up == TRUE)
    hsl_conf_write((int) arg,
		   PCI_COMMAND_STATUS_REG,
		   hsl_conf_read((int) arg, PCI_COMMAND_STATUS_REG) &
		   (PCI_COMMAND_SERR_ENABLE | PCI_COMMAND_PARITY_ENABLE));

  return;

 halt:
#define NO_HARDWARE_SHUTDOWN

#ifdef NO_HARDWARE_SHUTDOWN

  if (flush_interrupts == TRUE)
    log(LOG_ERR, "NO_HARDWARE_SHUTDOWN : INTERRUPT SHOULD BE IGNORED\n");
  else {
    log(LOG_ERR, "NO_HARDWARE_SHUTDOWN : INTERRUPT AFTER START-UP\n");
    wakeup(&flush_interrupts);
  }

  val = hsl_conf_read((int) arg,
		      PCI_COMMAND_STATUS_REG);
  hsl_conf_write((int) arg,
		 PCI_COMMAND_STATUS_REG,
		 (val & ( PCI_STATUS_PARITY_DETECT |
			  PCI_STATUS_PARITY_ERROR |
			  PCI_STATUS_SPECIAL_ERROR |
			  PCI_STATUS_MASTER_ABORT |
			  PCI_STATUS_MASTER_TARGET_ABORT |
			  PCI_STATUS_TARGET_TARGET_ABORT |
			  PCI_COMMAND_SERR_ENABLE |
			  PCI_COMMAND_PARITY_ENABLE)) |
		 /* Activate the Bus Master,
		    it could have been reset. */
		 PCI_COMMAND_MASTER_ENABLE);

  /* Clear every error events to prevent an infinite loop. */
  hsl_conf_write((int) arg,
		 PCIDDC_STATUS,
		 0xFFFFFFFFUL);

  return;

#else

  /* We don't want to be interrupted later :
     we clear the PCI-DDC mask register */
  hsl_conf_write((int) arg,
		 PCIDDC_STATUS_MASK_REGISTER,
		 0);

  val = hsl_conf_read((int) arg,
		      PCI_COMMAND_STATUS_REG);

  if (flush_interrupts == TRUE) {
    /* Ignore this interrupt (software fix for some interrupts that happen
       just after the warm up...) */

    log(LOG_ERR, "INTERRUPT IGNORED\n");

    hsl_conf_write((int) arg,
		   PCI_COMMAND_STATUS_REG,
		   val & ( PCI_STATUS_PARITY_DETECT |
			   PCI_STATUS_PARITY_ERROR |
			   PCI_STATUS_SPECIAL_ERROR |
			   PCI_STATUS_MASTER_ABORT |
			   PCI_STATUS_MASTER_TARGET_ABORT |
			   PCI_STATUS_TARGET_TARGET_ABORT |
			   PCI_COMMAND_SERR_ENABLE |
			   PCI_COMMAND_PARITY_ENABLE));
    wakeup(&flush_interrupts);

    return;
  }

  log(LOG_ERR, "HARDWARE ERROR CATCHED\n");

  /* Freeze PCI-DDC by clearing error bits and disabling the BUS MASTER */
  /* We MUST clear error bits to avoid an interrupt loop... */
  hsl_conf_write((int) arg,
		 PCI_COMMAND_STATUS_REG,
		 val & ( PCI_STATUS_PARITY_DETECT |
			 PCI_STATUS_SPECIAL_ERROR |
			 PCI_STATUS_MASTER_ABORT |
			 PCI_STATUS_MASTER_TARGET_ABORT |
			 PCI_STATUS_TARGET_TARGET_ABORT |
			 PCI_COMMAND_SERR_ENABLE |
			 PCI_COMMAND_PARITY_ENABLE) &
		 ~PCI_COMMAND_MASTER_ENABLE);

  /* Downgrad into Ethernet mode */
  /* This may not be sufficient : the return path may still be routed
     on the HSL network if no error occured on the other side */
  for (i = 0; i < MAX_NODES; i++)
    put_info[(int) arg].remote_hsl[i] = FALSE;

  /* Since we change the LPE, we must wake up processes waiting
     for entries in it */
  wakeup(&hslfreelpe_ident);

  return;
#endif
}
      /*******************************************************************/
#else /************************ PCIDDC 2nd Run ***************************/
      /*******************************************************************/

static void
put_interrupt_handler(arg)
                      void *arg;
{
  u_long pciddc_status, pci_status;
  u_long old_pciddc_status, old_pci_status;
  u_long val;
  lmi_entry_t *lmi_new_virt;
  mi_t mi;
  boolean_t should_halt = FALSE;
  int i;
#ifndef PUT_OPTIMIZE
  boolean_t should_clear_page_transmitted = FALSE;
  boolean_t should_clear_page_received = FALSE;
#endif

  /*#define AVOID_INFINITE_LOOP*/
#ifdef AVOID_INFINITE_LOOP
static int AVOID_LOOP_MAXINT = 100;
  /*   static int AVOID_LOOP_MAXINT = 10; */
#endif

#ifdef AVOID_INFINITE_LOOP
  if (AVOID_LOOP_MAXINT-- < 0) {
    /* INTERRUPT LOOP SUSPECTED */
    log(LOG_ERR,
	"INTERRUPT LOOP SUSPECTED, bus master cleared\n");

    val = hsl_conf_read((int) arg,
			PCI_COMMAND_STATUS_REG);
    hsl_conf_write((int) arg,
		   PCI_COMMAND_STATUS_REG,
		   val & ~PCI_COMMAND_MASTER_ENABLE);

    /* Perform a soft reset */
    log(LOG_ERR,
	"Performing a PCIDDC soft reset\n");
    hsl_conf_write((int) arg,
		   PCIDDC_COMMAND,
		   PCIDDC_SOFT_RESET);

/*     panic("interrupt loop suspected\n"); */

    return;
  }
#endif

  pciddc_status = hsl_conf_read((int) arg,
				PCIDDC_STATUS);
  pci_status = hsl_conf_read((int) arg,
			     PCI_COMMAND_STATUS_REG);


  /* On lit 9 fois de plus les registres pour corriger le bug des
     interruptions non attendues sans cause apparente à la 1ere lecture
     de pciddc_status. Ca permet d'ajouter une latence le temps
     que le registre indique la cause d'interruption. Mais il se pourrait,
     d'apres Cyril, que ce ne soit qu'une fausse solution, car en fait,
     après cette latence, c'est peut-etre une cause pour une autre
     émission qui apparait dans le registre... */
  /*
    for (i = 0; i < 9; i++) {
    pciddc_status = hsl_conf_read((int) arg,
				  PCIDDC_STATUS);
    pci_status = hsl_conf_read((int) arg,
			       PCI_COMMAND_STATUS_REG);
  }
  */

  /* Due to a bug of PCIDDC, the PCI_STATUS_PARITY_DETECT bit may
     be set but there is no interrupt generated for this condition.
     Thus, we decide to forget that the bit was set... */
  pci_status &= ~PCI_STATUS_PARITY_DETECT;

/* #define DUMP_PCIDDC_REGISTERS */
#ifdef DUMP_PCIDDC_REGISTERS
  log(LOG_ERR, "XXX pciddc_status=0x%x\nXXX pci_status=0x%x\n",
      (int) pciddc_status, (int) pci_status);
#endif


#if 0
  pciddc_status = hsl_conf_read((int) arg,
				PCIDDC_STATUS);
  pci_status = hsl_conf_read((int) arg,
			     PCI_COMMAND_STATUS_REG);
  log(LOG_ERR, "XXX0 pciddc_status=0x%x\nXXX0 pci_status=0x%x\n",
      (int) pciddc_status, (int) pci_status);


  val = hsl_conf_read((int) arg,
		      PCIDDC_STATUS);
  hsl_conf_write((int) arg,
		 PCIDDC_STATUS,
		 0x8000000);

  pciddc_status = hsl_conf_read((int) arg,
				PCIDDC_STATUS);
  pci_status = hsl_conf_read((int) arg,
			     PCI_COMMAND_STATUS_REG);
  log(LOG_ERR, "XXX2 pciddc_status=0x%x\nXXX2 pci_status=0x%x\n",
      (int) pciddc_status, (int) pci_status);

  val = hsl_conf_read((int) arg,
		      PCI_COMMAND_STATUS_REG);
  hsl_conf_write((int) arg,
		 PCI_COMMAND_STATUS_REG,
		 0);
  return;
#endif

#ifndef PUT_OPTIMIZE
 retry_unattended:
  /* Update unattended interrupts count */ 
  if (!(pci_status & PCI_STATUS_PARITY_ERROR
	/* de toutes facons le PARITY peut pas matcher vu qu'on l'enleve
	   auparavant (cf FORGET...), mais en principe pas de probleme,
	   car c'est justement enleve car il n'y a pas d'interrupt
	   en realite */) &&
      !(pciddc_status & (PCIDDC_STATUS_PAGE_TRANSMITTED |
			 PCIDDC_STATUS_PAGE_RECEIVED |
			 PCIDDC_STATUS_PAGE_TRANSMITTED_4096 |
			 PCIDDC_STATUS_PAGE_TRANSMITTED_256 |
			 PCIDDC_STATUS_PAGE_TRANSMITTED_16 |
			 PCIDDC_STATUS_PAGE_RECEIVED_4096 |
			 PCIDDC_STATUS_PAGE_RECEIVED_256 |
			 PCIDDC_STATUS_PAGE_RECEIVED_16 |
			 PCIDDC_STATUS_EEP_ERROR |
			 PCIDDC_STATUS_CRC_HEADER_ERROR |
			 PCIDDC_STATUS_CRC_DATA_ERROR |
			 PCIDDC_STATUS_EP_ERROR |
			 PCIDDC_STATUS_TIMEOUT_ERROR |
			 PCIDDC_STATUS_R3_STATUS_ERROR |
			 PCIDDC_STATUS_PAGE_TRANSMITTED_OF_ERROR |
			 PCIDDC_STATUS_PAGE_RECEIVED_OF_ERROR |
			 PCIDDC_STATUS_SENT_PACKET_OF_ERROR |
			 PCIDDC_STATUS_RECEIVED_PACKET_OF_ERROR |
			 PCIDDC_STATUS_ILLEGAL_LRM_ACCESS_ERROR |
			 PCIDDC_STATUS_LMI_OF_ERROR))) {
    driver_stats.unattended_interrupts++;

#ifdef WARN_FOR_UNATTENDED_INTERRUPT
    log(LOG_ERR,
	"UNATTENDED INTERRUPT\n");
    log(LOG_ERR, "PCIDDC: OLD values : pciddc_status=0x%x\n        pci_status=0x%x\n",
	(int) pciddc_status, (int) pci_status);
    pciddc_status = hsl_conf_read((int) arg,
				  PCIDDC_STATUS);
    pci_status = hsl_conf_read((int) arg,
			       PCI_COMMAND_STATUS_REG);
    log(LOG_ERR, "PCIDDC: pciddc_status=0x%x\n        pci_status=0x%x\n",
	(int) pciddc_status, (int) pci_status);
    return;
#else
    old_pciddc_status = pciddc_status;
    old_pci_status = pci_status;
    pciddc_status = hsl_conf_read((int) arg,
				  PCIDDC_STATUS);
    pci_status = hsl_conf_read((int) arg,
			       PCI_COMMAND_STATUS_REG);
    /* Due to a bug of PCIDDC, the PCI_STATUS_PARITY_DETECT bit may
       be set whereas there is no interrupt generated for this condition.
       Thus, we decide to forget that the bit was set... */
    pci_status &= ~PCI_STATUS_PARITY_DETECT;

    if (pciddc_status != old_pciddc_status ||
	pci_status != old_pci_status) {
      driver_stats.unattended_interrupts--;
      /* Due to a bug of PCIDDC, the PCI_STATUS_PARITY_DETECT bit may
	 be set whereas there is no interrupt generated for this condition.
	 Thus, we decide to forget that the bit was set... */
      goto retry_unattended;
    }
    return;
#endif
  }
#endif

#ifndef PUT_OPTIMIZE
  /* Avoid interrupt loop */
  if (pciddc_status & PCIDDC_STATUS_PAGE_TRANSMITTED)
    should_clear_page_transmitted = TRUE;
  if (pciddc_status & PCIDDC_STATUS_PAGE_RECEIVED)
    should_clear_page_received = TRUE;
  if (should_clear_page_transmitted == FALSE &&
      should_clear_page_received == FALSE) {

#ifdef AVOID_LINK_ERROR
    if (!(pciddc_status & PCIDDC_STATUS_EEP_ERROR)) {
#endif
      log(LOG_ERR,
	  "UNATTENDED INTERRUPT : not transmitted nor received page\n");
      /* On ne devrait pas mettre la ligne qui suit, car il y a des
	 cas où on ne traite ni PT, ni PR, et où on a une cause d'interruption
	 qui n'impose pas d'arrêter PCIDDC,
	 par ex. PCIDDC_STATUS_PAGE_RECEIVED_OF_ERROR.
	 On met néanmoins cette ligne car on veut comprendre ce qui
	 se passe précisément, afin de débugger PUT. */
#ifdef AVOID_LINK_ERROR
    }
#endif

    should_halt = TRUE;
  }
#endif

#ifdef DEBUG_HSL
  log(LOG_DEBUG,
      "put_interrupt_handler(minor=%d)\n", (int) arg);
#endif

#ifdef MPC_STATS
  driver_stats.calls_put_interrupt++;
#endif

  /*
   * Evaluation of pci_conf_write (pci_conf_read should take longer time...) :
   * the configuration write costs ~1.7µs. If we replace it by the following method,
   * we save 0.4µs. The cost of the io write (outl) seems to be about 0.3 or 0.4µs.
   * 3 outl are needed to perform 1 conf_write.
   * Thoses results were computed on a P100 with an HP Logic Analyser.
   * For further informations, please feel free to contact F. Wasjbürt and A. Fenyö.
   *

  BENCH for the HP Logic Analyser :
  {
    struct pcibus *p;
    int s;

    s = splhigh();
    p = getpcibus();
    p->pb_write(put_info[(int) arg].hsl_tag, 0xfc, 0x1234567);
    p->pb_write(put_info[(int) arg].hsl_tag, 0xfc, 0x1234567);
    p->pb_write(put_info[(int) arg].hsl_tag, 0xfc, 0x1234567);
    p->pb_write(put_info[(int) arg].hsl_tag, 0xfc, 0x1234567);
    p->pb_write(put_info[(int) arg].hsl_tag, 0xfc, 0x1234567);
    splx(s);
  }
  */

  /* Check hardware errors */

#ifndef PUT_OPTIMIZE
#ifdef AVOID_LINK_ERROR
  if (!(pciddc_status & PCIDDC_STATUS_EEP_ERROR)) {
#endif
    if (!(pci_status & PCI_COMMAND_MASTER_ENABLE))
      log(LOG_ERR,
	  "hardware warning: BUS MASTER down\n");
#ifdef AVOID_LINK_ERROR
  }
#endif
#endif

  if ((pciddc_status & (PCIDDC_STATUS_EEP_ERROR |
			PCIDDC_STATUS_CRC_HEADER_ERROR |
			PCIDDC_STATUS_CRC_DATA_ERROR |
			PCIDDC_STATUS_EP_ERROR |
			PCIDDC_STATUS_TIMEOUT_ERROR |
			PCIDDC_STATUS_R3_STATUS_ERROR |
			PCIDDC_STATUS_PAGE_TRANSMITTED_OF_ERROR |
			PCIDDC_STATUS_PAGE_RECEIVED_OF_ERROR |
			PCIDDC_STATUS_SENT_PACKET_OF_ERROR |
			PCIDDC_STATUS_RECEIVED_PACKET_OF_ERROR |
			PCIDDC_STATUS_ILLEGAL_LRM_ACCESS_ERROR |
			PCIDDC_STATUS_LMI_OF_ERROR)) ||
      (pci_status & (PCI_STATUS_PARITY_DETECT |
		     PCI_STATUS_SPECIAL_ERROR |
		     PCI_STATUS_MASTER_ABORT |
		     PCI_STATUS_MASTER_TARGET_ABORT |
		     PCI_STATUS_TARGET_TARGET_ABORT |
		     PCI_STATUS_PARITY_ERROR))) {

#ifdef AVOID_LINK_ERROR
    if (!(pciddc_status & PCIDDC_STATUS_EEP_ERROR)) {
#endif
      log(LOG_ERR,
	  "HARDWARE ERROR:\n");

      log(LOG_ERR,
	  "                         pciddc status: 0x%x\n", (int) pciddc_status);
#ifdef AVOID_LINK_ERROR
    }
#endif

    /* Check error bits in the pciddc status register */
#ifndef AVOID_LINK_ERROR
    if (pciddc_status & PCIDDC_STATUS_EEP_ERROR)
#else
    if (pciddc_status & (PCIDDC_STATUS_EEP_ERROR | PCIDDC_STATUS_EP_ERROR))
#endif
      {
#ifndef AVOID_LINK_ERROR
      log(LOG_ERR,
	  "hardware error: Exceptional End of Packet Error\n");
#else
      if (pciddc_status & PCIDDC_STATUS_EEP_ERROR)
	log(LOG_ERR,
	    "hardware error: Exceptional End of Packet Error\n");
      if (pciddc_status & PCIDDC_STATUS_EP_ERROR)
	log(LOG_ERR,
	    "hardware error: End of Packet Error\n");
#endif
      should_halt = TRUE;

#ifdef AVOID_LINK_ERROR
      if (flush_interrupts != TRUE) {
	lmi_entry_t *lmi_new_p;

	/* Check calibration */
	if ((hsl_conf_read((int) arg, PCIDDC_STATUS) & 0xff) != 0x3)
	  log(LOG_ERR, "RCube : Warning, CALIBRATION LOST !!!!!\n");

	/* Clear the error event */
	if (pciddc_status & PCIDDC_STATUS_EEP_ERROR)
	  hsl_conf_write((int) arg,
			 PCIDDC_STATUS,
			 PCIDDC_STATUS_EEP_ERROR);
	if (pciddc_status & PCIDDC_STATUS_EP_ERROR)
	  hsl_conf_write((int) arg,
			 PCIDDC_STATUS,
			 PCIDDC_STATUS_EP_ERROR);

	avoid_stage = 1;

#if 0
	if (hsl_conf_read((int) arg, PCIDDC_LMI_NEW) ==
	    hsl_conf_read((int) arg, PCIDDC_LMI_CURRENT))
	  log(LOG_ERR,
	      "AVOID_LINK_ERROR: LMI empty\n");
#endif

	lmi_new_p = (lmi_entry_t *)
	  (hsl_conf_read((int) arg, PCIDDC_LMI_NEW) -
	   (u_long) put_info[(int) arg].hsl_contig_space_phys +
	   (u_long) put_info[(int) arg].hsl_contig_space);

	/* Set the BUS MASTER up */
	val = hsl_conf_read((int) arg,
			    PCI_COMMAND_STATUS_REG);
	hsl_conf_write((int) arg,
		       PCI_COMMAND_STATUS_REG,
		       val | PCI_COMMAND_MASTER_ENABLE);

	log(LOG_ERR,
	    "AVOID_LINK_ERROR: LMI_NEW index = 0x%x\n",
	    lmi_new_p - (lmi_entry_t *) put_info[(int) arg].hsl_lmi);

	avoid_v0_entry_n = avoid_v0_entry_nn = lmi_new_p;
	if (lmi_new_p-- ==
	    (lmi_entry_t *) put_info[(int) arg].hsl_lmi)
	  lmi_new_p = ((lmi_entry_t *) put_info[(int) arg].hsl_lmi) +
	    HSL_LMI_SIZE - 1;
	avoid_v0 = (lmi_new_p->control>>8) & 0xffff;
	avoid_v0_entry = lmi_new_p;
	if (++avoid_v0_entry_nn ==
	    ((lmi_entry_t *) put_info[(int) arg].hsl_lmi) + HSL_LMI_SIZE)
	  avoid_v0_entry_nn = (lmi_entry_t *) put_info[(int) arg].hsl_lmi;

	/* Wakeup hslclient */
	wakeup(&hslgetlpe_ident);

	return;
      }
#endif
     }

    if (pciddc_status & PCIDDC_STATUS_CRC_HEADER_ERROR) {
      log(LOG_ERR,
	  "hardware error: CRC Header Error\n");
      should_halt = TRUE;
    }

    if (pciddc_status & PCIDDC_STATUS_CRC_DATA_ERROR) {
      log(LOG_ERR,
	  "hardware error: CRC Data Error\n");
      should_halt = TRUE;
    }

    if (pciddc_status & PCIDDC_STATUS_EP_ERROR) {
      log(LOG_ERR,
	  "hardware error: End of Packet Error\n");
      should_halt = TRUE;

#ifdef AVOID_LINK_ERROR
      if (flush_interrupts != TRUE) {
	/* Clear the BUS MASTER */
	val = hsl_conf_read((int) arg,
			    PCI_COMMAND_STATUS_REG);
	hsl_conf_write((int) arg,
		       PCI_COMMAND_STATUS_REG,
		       val & ~PCI_COMMAND_MASTER_ENABLE);
	/* Clear the error event */
	hsl_conf_write((int) arg,
		       PCIDDC_STATUS,
		       PCIDDC_STATUS_EP_ERROR);
	return;
      }
#endif
    }

    if (pciddc_status & PCIDDC_STATUS_TIMEOUT_ERROR) {
      log(LOG_ERR,
	  "hardware error: Timeout Error\n");
      should_halt = TRUE;

#ifdef AVOID_LINK_ERROR
      if (flush_interrupts != TRUE) {
	/* Clear the error event */
	hsl_conf_write((int) arg,
		       PCIDDC_STATUS,
		       PCIDDC_STATUS_TIMEOUT_ERROR);
	return;
      }
#endif
    }

    if (pciddc_status & PCIDDC_STATUS_R3_STATUS_ERROR) {
      log(LOG_ERR,
	  "hardware error: R3 Status Error\n");
      should_halt = TRUE;
    }

    if (pciddc_status & PCIDDC_STATUS_PAGE_TRANSMITTED_OF_ERROR) {
      log(LOG_ERR,
	  "hardware error: Page Transmitted Overflow Error\n");
      /* This is not really an error, but we want to know
	 when this condition appears */
      /* should_halt = FALSE; */

      /* Clear the error event */
      hsl_conf_write((int) arg,
		     PCIDDC_STATUS,
		     PCIDDC_STATUS_PAGE_TRANSMITTED_OF_ERROR);
    }

    if (pciddc_status & PCIDDC_STATUS_PAGE_RECEIVED_OF_ERROR) {
      log(LOG_ERR,
	  "hardware error: Page Received Overflow Error\n");
      /* This is not really an error, but we want to know
	 when this condition appears */
      /* should_halt = FALSE; */

      /* Clear the error event */
      hsl_conf_write((int) arg,
		     PCIDDC_STATUS,
		     PCIDDC_STATUS_PAGE_RECEIVED_OF_ERROR);
    }

    if (pciddc_status & PCIDDC_STATUS_SENT_PACKET_OF_ERROR) {
      log(LOG_ERR,
	  "hardware error: Sent Packet Overflow Error\n");
      /* This is not always an error, but we want to know
	 when this condition appears */
      /* should_halt = FALSE; */ /* If the LRM was handled correctly, we
	                            should halt for this error */

      /* Clear the error event */
      hsl_conf_write((int) arg,
		     PCIDDC_STATUS,
		     PCIDDC_STATUS_SENT_PACKET_OF_ERROR);
    }

    if (pciddc_status & PCIDDC_STATUS_RECEIVED_PACKET_OF_ERROR) {
      log(LOG_ERR,
	  "hardware error: Received Packet Overflow Error\n");
      /* This is not always an error, but we want to know
	 when this condition appears */
      /* should_halt = FALSE; */ /* If the LRM was handled correctly, we
			            should halt for this error */

      /* Clear the error event */
      hsl_conf_write((int) arg,
		     PCIDDC_STATUS,
		     PCIDDC_STATUS_RECEIVED_PACKET_OF_ERROR);
    }

    if (pciddc_status & PCIDDC_STATUS_ILLEGAL_LRM_ACCESS_ERROR) {
      log(LOG_ERR,
	  "hardware error: Illegal LRM Access Error\n");
      should_halt = TRUE;
    }

    /* Check error bits in the pci command and status register */

    log(LOG_ERR,
	"                                                          c&s: 0x%x\n", (int) pci_status);

    if (pci_status & PCI_STATUS_PARITY_DETECT) {
      /* Ce cas ne peut pas arriver car au debut du handler,
	 on "oublie" que ce bit est positionne si des fois il l'etait */
      log(LOG_ERR,
	  "hardware error: Detected Parity Error\n");
      should_halt = TRUE;
    }

    if (pci_status & PCI_STATUS_SPECIAL_ERROR) {
      log(LOG_ERR,
	  "hardware error: Signaled System Error\n");
      should_halt = TRUE;
    }

    if (pci_status & PCI_STATUS_MASTER_ABORT) {
      log(LOG_ERR,
	  "hardware error: Received Master Abort Error\n");
      should_halt = TRUE;
    }

    if (pci_status & PCI_STATUS_MASTER_TARGET_ABORT) {
      log(LOG_ERR,
	  "hardware error: Received Target Abort Error\n");
      should_halt = TRUE;
    }

    if (pci_status & PCI_STATUS_TARGET_TARGET_ABORT) {
      log(LOG_ERR,
	  "hardware error: Signaled Target Abort Error\n");
      should_halt = TRUE;
    }

    if (pci_status & PCI_STATUS_PARITY_ERROR) {
      log(LOG_ERR,
	  "hardware error: Data Parity Error Detected\n");
      should_halt = TRUE;
    }

    if (pciddc_status & PCIDDC_STATUS_LMI_OF_ERROR) {
      log(LOG_ERR,
	  "hardware error: LMI Overflow Error\n");
      /* should_halt = FALSE; */

      /* Clear the error event */
      hsl_conf_write((int) arg,
		     PCIDDC_STATUS,
		     /* Due to a bug in PCIDDC, if we want to clear
			PCIDDC_STATUS_LMI_OF_ERROR, we need to clear
			PCIDDC_STATUS_ILLEGAL_LRM_ACCESS_ERROR, and
			this will clear
			PCIDDC_STATUS_ILLEGAL_LRM_ACCESS_ERROR and in
			the same time PCIDDC_STATUS_LMI_OF_ERROR */
		     PCIDDC_STATUS_ILLEGAL_LRM_ACCESS_ERROR /* PCIDDC_STATUS_LMI_OF_ERROR */);

#if 0
#ifdef AVOID_LINK_ERROR
      /* Clear the BUS MASTER */
      val = hsl_conf_read((int) arg,
			  PCI_COMMAND_STATUS_REG);
      hsl_conf_write((int) arg,
		     PCI_COMMAND_STATUS_REG,
		     val & ~PCI_COMMAND_MASTER_ENABLE);
      return;
#endif
#endif
    }
  }

  if (should_halt == TRUE) {

    log(LOG_ERR,
	"Should halt PCIDDC\n");

    if (flush_interrupts == TRUE) {
      log(LOG_ERR,
	  "NO_HARDWARE_SHUTDOWN : INTERRUPT SHOULD BE IGNORED\n");

      val = hsl_conf_read((int) arg,
			  PCIDDC_STATUS);
      hsl_conf_write((int) arg,
		     PCIDDC_STATUS,
		     val & (PCI_STATUS_PARITY_DETECT |
			    PCI_STATUS_SPECIAL_ERROR |
			    PCI_STATUS_MASTER_ABORT |
			    PCI_STATUS_MASTER_TARGET_ABORT |
			    PCI_STATUS_TARGET_TARGET_ABORT |
			    PCI_STATUS_PARITY_ERROR));

      val = hsl_conf_read((int) arg,
			  PCI_COMMAND_STATUS_REG);
      hsl_conf_write((int) arg,
		     PCI_COMMAND_STATUS_REG,
		     (val & (PCI_STATUS_PARITY_DETECT |
			     PCI_STATUS_SPECIAL_ERROR |
			     PCI_STATUS_MASTER_ABORT |
			     PCI_STATUS_MASTER_TARGET_ABORT |
			     PCI_STATUS_TARGET_TARGET_ABORT |
			     PCI_STATUS_PARITY_ERROR)) |
		     /* Activate the Bus Master,
			it could have been reset. */
		     PCI_COMMAND_MASTER_ENABLE);

      return;
    }

#if 0
    /* Downgrade into Ethernet mode */
    /* This may not be sufficient : the return path may still be routed
       on the HSL network if no error occured on the other side */
    for (i = 0; i < MAX_NODES; i++)
      put_info[(int) arg].remote_hsl[i] = FALSE;
#endif

    /* We need to clear the BUS MASTER to avoid an infinite loop,
       since some error sources don't do it
       (but should -> software fix of hardware bug) */
    val = hsl_conf_read((int) arg,
			PCI_COMMAND_STATUS_REG);
    hsl_conf_write((int) arg,
		   PCI_COMMAND_STATUS_REG,
		   val & ~PCI_COMMAND_MASTER_ENABLE);

    /* Since we change the LPE, we must wake up processes waiting
       for entries in it */ /* ??? */
    wakeup(&hslfreelpe_ident);

    /* Enlever le Bus Master semble parfois etre inefficace -> on fait un soft reset... */
    /* Un panic (sans soft reset) semble etre aussi suffisant */
    /*     panic("interrupt loop suspected\n"); */

    /* Perform a soft reset */
    log(LOG_ERR,
	"Performing a PCIDDC soft reset\n");
    hsl_conf_write((int) arg,
		   PCIDDC_COMMAND,
		   PCIDDC_SOFT_RESET);

    return;
  }

  /************************************************************/
  /* TRANSMITTED PAGES HANDLING                               */
  /************************************************************/

  /* ATTENTION : le registre LPE_CURRENT de PCIDDC est incrémenté au
     début du traitement d'une entrée de LPE, et non à la fin de
     l'émission de données correspondante. Pour acquitter les entrées,
     nous nous proposons ici, pour chaque entrée de LPE LUE par PCIDDC
     depuis le derniere traitement, et comportant le drapeau NOS, de
     successivement tester dans PCIDDC la cause PAGE_TRANSMITTED, puis
     si elle est active, de traiter l'entrée en l'acquittant et en
     appelant le layer correspondant. On pourrait optimiser ce
     processus en cherchant dans le sous ensemble des entrées
     comportant NOS, celle qui est la plus récente, et traiter TOUTES
     les entrées précédentes, puis de faire un traitement particulier
     pour la dernière (test de PAGE_TRANSMITTED pour décider de la
     traiter ou pas). */

/*
log(LOG_DEBUG, "IRQ: current=0x%x,new=0x%x,softnew=0x%x\n",
put_info[(int) arg].lpe_current_virt,
put_info[(int) arg].lpe_new_virt,
put_info[(int) arg].lpe_softnew_virt);
*/

  /* We should call put_flush_lpe() instead of inserting this block.
     It's not possible because there is a very little difference
     between this block and put_flush_lpe() :
     the should_clear_page_transmitted flag is handled here */
  {
    lpe_entry_t *old_current_virt;

    old_current_virt = put_info[(int) arg].lpe_current_virt;

    /* Update our copy of the LPE CURRENT pointer */
    put_info[(int) arg].lpe_current_virt = (lpe_entry_t *)
      (hsl_conf_read((int) arg, PCIDDC_LPE_CURRENT) -
       (u_long) put_info[(int) arg].hsl_contig_space_phys +
       (u_long) put_info[(int) arg].hsl_contig_space);

    /* A CET ENDROIT : ON DOIT AVOIR put_info[(int) arg].lpe_current_virt == put_info[(int) arg].lpe_new_virt A CHAQUE BLOC */
    /*if (put_info[(int) arg].lpe_current_virt == put_info[(int) arg].lpe_new_virt)
log(LOG_DEBUG, "IRQ2: current=0x%x,new=0x%x,softnew=0x%x\n",
put_info[(int) arg].lpe_current_virt,
put_info[(int) arg].lpe_new_virt,
put_info[(int) arg].lpe_softnew_virt);*/

    if (old_current_virt != put_info[(int) arg].lpe_current_virt)
      /* Signal that some new LPE entries will soon be free */
      wakeup(&hslfreelpe_ident);

    while (old_current_virt != put_info[(int) arg].lpe_current_virt) {

      /* here, we should/could only test NOS_MASK, but a bug in PCIDDC
	 forces us to set LMP when NOS is set, to get an interrupt
	 (and have the counter incremented : to be checked) */
      if ((old_current_virt->control & (NOS_MASK | LMP_MASK)) ==
	  (NOS_MASK | LMP_MASK)) {
	u_long temp_pciddc_status;

	temp_pciddc_status = hsl_conf_read((int) arg,
					   PCIDDC_STATUS);

	/* Warning : if PAGE_TRANSMITTED is not set, this LPE entry is not completed */
	/* C'est la condition de sortie de cette boucle */
	if (!(temp_pciddc_status & PCIDDC_STATUS_PAGE_TRANSMITTED)) {
	  put_info[(int) arg].lpe_current_virt = old_current_virt;
	  break;
	}

	/* Acknowledge the entry */
	/* A method with better performances would be to acknowledge
	   entries 16, 256 or 4096 at a time... */
	hsl_conf_write((int) arg,
		       PCIDDC_STATUS,
		       PCIDDC_STATUS_PAGE_TRANSMITTED);

#ifndef PUT_OPTIMIZE
	should_clear_page_transmitted = FALSE;
#endif

#if defined(LPE_SLOW) || defined(LPE_BURST)
	/* Recover the real NOS bit value
	   (in order to fix a bug in PCI-DDC) */
	if (RESERVED_ISSET(old_current_virt->control))
	  old_current_virt->control |=
	    NOS_MASK;
	else
	  old_current_virt->control &=
	    ~NOS_MASK;
#endif

#ifdef LPE_SLOW
	/* The DMA is terminated */
	put_info[(int) arg].lpe_loaded = FALSE;
#endif

#if defined(LPE_SLOW) || defined(LPE_BURST)
	/* Call the upper layer interrupt handler if the NOS bit was
	   effectively set at the put_add_entry() time */
	if (RESERVED_ISSET(old_current_virt->control)) {
#endif

	  /* Call the upper layer interrupt handler */
	  mi = MI_PART(old_current_virt->control);
	  for (i = 0; i < MAX_SAP; i++) {
	    void (*fct) __P((int,
			     mi_t,
			     lpe_entry_t));

	    fct = put_info[(int) arg].IT_sent[i];

	    if (fct &&
		mi >= put_info[(int) arg].mi_space[i].mi_start &&
		mi < put_info[(int) arg].mi_space[i].mi_start +
		put_info[(int) arg].mi_space[i].mi_size) {
#ifdef AVOID_LINK_ERROR
	      register_interrupt(interrupt_table_LPE,
				 (old_current_virt->control & 0x00ffff00)>>8,
				 (void (*)()) fct,
				 *old_current_virt,
				 TRUE,
				 (u_long) arg,
				 (u_long) mi);
	      fct((int) arg,
		  mi,
		  *old_current_virt);
#else
	      fct((int) arg,
		  mi,
		  *old_current_virt);
#endif
	      break;
	    }
	  }
#if defined(LPE_SLOW) || defined(LPE_BURST)
	}
#endif
      }
	
      /* Continue with the next LPE entry to acknowledge */
      if (++old_current_virt ==
	  ((lpe_entry_t *) put_info[(int) arg].hsl_lpe) + HSL_LPE_SIZE)
	old_current_virt = (lpe_entry_t *) put_info[(int) arg].hsl_lpe;
    }

    /************************************************************/

#if !defined(LPE_SLOW) && !defined(LPE_BURST)
    if (put_info[(int) arg].lpe_new_virt != put_info[(int) arg].lpe_softnew_virt)
      panic("Integrity check error inside interrupt handler\n");
#endif

#ifdef LPE_SLOW
    if (put_info[(int) arg].lpe_loaded == FALSE &&
	put_info[(int) arg].lpe_new_virt != put_info[(int) arg].lpe_softnew_virt) {

      /* Increment the LPE_NEW register */
      if (++put_info[(int) arg].lpe_new_virt ==
	  ((lpe_entry_t *) put_info[(int) arg].hsl_lpe) + HSL_LPE_SIZE)
	put_info[(int) arg].lpe_new_virt =
	  (lpe_entry_t *) put_info[(int) arg].hsl_lpe;

      hsl_conf_write((int) arg,
		     PCIDDC_LPE_NEW,
		     /* We could use PCIDDC_VIRT2PHYS()
			instead of vtophys() */
		     vtophys(put_info[(int) arg].lpe_new_virt));

      put_info[(int) arg].lpe_loaded = TRUE;
    }
#endif

#ifdef LPE_BURST
    {
      int block_current;
      int block_softnew;
      lpe_entry_t *next_new;

      /* Divide the LPE in several blocks of size LPE_BURST_LIMIT */
      block_current = (put_info[(int) arg].lpe_current_virt -
		       (lpe_entry_t *) put_info[(int) arg].hsl_lpe)
	/ LPE_BURST_LIMIT;
      block_softnew = (put_info[(int) arg].lpe_softnew_virt -
		       (lpe_entry_t *) put_info[(int) arg].hsl_lpe)
	/ LPE_BURST_LIMIT;

      /* If the current pointer is not in the same block than the
	 softnew pointer, let the next new value be the beginning of
	 the block just after the one of the current pointer. */
      if (block_current != block_softnew) {
	next_new = ((lpe_entry_t *) put_info[(int) arg].hsl_lpe) +
	  (block_current + 1) * LPE_BURST_LIMIT;
	if (next_new ==
	    ((lpe_entry_t *) put_info[(int) arg].hsl_lpe) + HSL_LPE_SIZE)
	  next_new =
	    (lpe_entry_t *) put_info[(int) arg].hsl_lpe;
      }
      else
	next_new = put_info[(int) arg].lpe_softnew_virt;

      /* Update the LPE_NEW register */
      if (next_new != put_info[(int) arg].lpe_new_virt) {
	put_info[(int) arg].lpe_new_virt = next_new;

	hsl_conf_write((int) arg,
		       PCIDDC_LPE_NEW,
		       /* We could use PCIDDC_VIRT2PHYS()
			  instead of vtophys() */
		       vtophys(put_info[(int) arg].lpe_new_virt));
      }
    }
#endif

  }


  /************************************************************/
  /* RECEIVED MESSAGES HANDLING                               */
  /************************************************************/

  /* A noter : il ne faut pas panacher des messages avec NOR sans LMI
     et des messages avec NOR et LMI : le driver aurait alors
     l'impossibilité de gérer les IRQ et la LMI : il ne pourrait
     pas associer un PCIDDC_STATUS_PAGE_RECEIVED à l'entrée de LMI
     correspondante, car cette entrée pourrait ne pas exister...
     Par contre, on peut panacher des LMI avec NOR et des LMI sans NOR. */

    if (pciddc_status & PCIDDC_STATUS_PAGE_RECEIVED) {
#ifdef AVOID_LINK_ERROR
      boolean_t dont_call_upper_layer;
      u_long control;
#endif

    /* Convert LMI_NEW pointer into kernel virtual address space */
    lmi_new_virt = (lmi_entry_t *)
      (hsl_conf_read((int) arg, PCIDDC_LMI_NEW) -
       (u_long) put_info[(int) arg].hsl_contig_space_phys +
       (u_long) put_info[(int) arg].hsl_contig_space);

    /* Treat and acknowledge each entry in the LMI */
    while (put_info[(int) arg].lmi_current_virt != lmi_new_virt) {

      /* Si on a une entrée de LMI sans NOR, on la signale néanmoins */
      /* Acknowledge the entry */
      /* A method with better performances would be to acknowledge
	 entries 16, 256 or 4096 at a time... */
      if (put_info[(int) arg].lmi_current_virt->control & NOR_MASK)
	hsl_conf_write((int) arg,
		       PCIDDC_STATUS,
		       PCIDDC_STATUS_PAGE_RECEIVED);
#ifndef PUT_OPTIMIZE
	should_clear_page_received = FALSE;
#endif

#ifdef AVOID_LINK_ERROR
	if (avoid_stage)
	  control = put_info[(int) arg].lmi_current_virt->control;

	if ((avoid_stage == 2) &&
	    (put_info[(int) arg].lmi_current_virt ==
	     avoid_v0_entry_nn)) avoid_stage = 3;

	if ((avoid_stage == 1) &&
	    (put_info[(int) arg].lmi_current_virt ==
	     avoid_v0_entry_n)) avoid_stage = 2;

	dont_call_upper_layer = FALSE;

	if (avoid_stage == 4) {
	  if (((control>>8) & 0xffff) ==
	      ((avoid_v0_entry_n->control>>8) & 0xffff))
	    log(LOG_ERR,
		"AVOID_LINK_ERROR: new LMI[v0+1] received (v1=0x%lx)\n",
		(control>>8) & 0xffff);

	  if (((control>>8) & 0xffff) ==
	      ((avoid_v0_entry_nn->control>>8) & 0xffff)) {
	    log(LOG_ERR,
		"AVOID_LINK_ERROR: new LMI[v0+2] received (v2=0x%lx)\n",
		(control>>8) & 0xffff);
	    avoid_stage = 0;
	  }
	}

	if ((avoid_stage == 2 || avoid_stage == 3) &&
	    (((control>>8) & 0xffff) ==
	     ((avoid_v0_entry_n->control>>8) & 0xffff))) {
	  log(LOG_ERR,
	      "AVOID_LINK_ERROR: initial LMI[v0+1] received (v1=0x%lx)\n",
	      (control>>8) & 0xffff);
	  dont_call_upper_layer = TRUE;
	}

	if ((avoid_stage == 3) &&
	    (((control>>8) & 0xffff) ==
	     ((avoid_v0_entry_nn->control>>8) & 0xffff))) {
	  log(LOG_ERR,
	      "AVOID_LINK_ERROR: initial LMI[v0+2] received (v2=0x%lx)\n",
	      (control>>8) & 0xffff);
	  dont_call_upper_layer = TRUE;
	}

	if ((avoid_stage == 3) &&
	    (MI_PART(control) ==
	     put_get_mi_start((int) arg, hsltestput_layer) + 5)) {
	  log(LOG_ERR,
	      "AVOID_LINK_ERROR: random message received (vx=0x%lx)\n",
	      (control>>8) & 0xffff);
	  avoid_stage = 4;
	}
#endif

	/* Call the upper layer interrupt handler */

#ifdef AVOID_LINK_ERROR
	if (dont_call_upper_layer == FALSE) {
#endif
	  mi = MI_PART(put_info[(int) arg].lmi_current_virt->control);
	  for (i = 0; i < MAX_SAP; i++) {
	    void (*fct) __P((int,
			     mi_t,
			     u_long,
			     u_long));

	    fct = put_info[(int) arg].IT_received[i];

	    if (fct && mi >= put_info[(int) arg].mi_space[i].mi_start &&
		mi < put_info[(int) arg].mi_space[i].mi_start +
		put_info[(int) arg].mi_space[i].mi_size) {
#ifdef AVOID_LINK_ERROR
	      register_interrupt(interrupt_table_LMI,
				 (put_info[(int) arg].lmi_current_virt->control & 0x00ffff00)>>8,
				 (void (*)()) fct,
				 unused_lpe_entry,
				 FALSE,
				 (u_long) arg,
				 (u_long) MI_PART(put_info[(int) arg].lmi_current_virt->control),
				 (u_long) (SM_ISSET(put_info[(int) arg].lmi_current_virt->control) ?
					   (u_int) put_info[(int) arg].lmi_current_virt->data1 : 0),
				 (u_long) (SM_ISSET(put_info[(int) arg].lmi_current_virt->control) ?
					   (u_int) put_info[(int) arg].lmi_current_virt->data2 : 0));
#if 0
	      fct((int) arg,
		  MI_PART(put_info[(int) arg].lmi_current_virt->control),
		  SM_ISSET(put_info[(int) arg].lmi_current_virt->control) ?
		  (u_int) put_info[(int) arg].lmi_current_virt->data1 : 0,
		  SM_ISSET(put_info[(int) arg].lmi_current_virt->control) ?
		  (u_int) put_info[(int) arg].lmi_current_virt->data2 : 0);
#endif
#else
	      fct((int) arg,
		  MI_PART(put_info[(int) arg].lmi_current_virt->control),
		  SM_ISSET(put_info[(int) arg].lmi_current_virt->control) ?
		  (u_int) put_info[(int) arg].lmi_current_virt->data1 : 0,
		  SM_ISSET(put_info[(int) arg].lmi_current_virt->control) ?
		  (u_int) put_info[(int) arg].lmi_current_virt->data2 : 0);
#endif
	      break;
	    }
	  }
#ifdef AVOID_LINK_ERROR
	}
#endif

      /* Increment lmi_current */
      if (++put_info[(int) arg].lmi_current_virt ==
	  ((lmi_entry_t *) put_info[(int) arg].hsl_lmi) + HSL_LMI_SIZE)
	put_info[(int) arg].lmi_current_virt =
	  (lmi_entry_t *) put_info[(int) arg].hsl_lmi;
    }

    /* Update the new lmi_current value in the chip */
    /* Due to a bug in PCI-DDC,
       LMI_NEW & LMI_CURRENT are reverted as
       they are written... */
    hsl_conf_write((int) arg,
		   PCIDDC_LMI_NEW /* PCIDDC_LMI_CURRENT */,
		   /* We could use PCIDDC_VIRT2PHYS()
		      instead of vtophys() */
		   vtophys(put_info[(int) arg].lmi_current_virt));

#ifdef AVOID_LINK_ERROR
    if (!avoid_stage && interrupt_table_LMI->nentries)
      do_interrupts(interrupt_table_LMI,
		    interrupt_table_LMI->interrupt[interrupt_table_LMI->nentries - 1].index);
#endif

    /*
     * Here, we should examine the PAGE RECEIVED bit in the
     * PCI-DDC status register and, if set,
     * try again the treatment of messages received,
     * in order to deal with messages arrived during the first
     * treatment. This may increase performances...
     */
    }

#ifndef PUT_OPTIMIZE
    if ((should_clear_page_transmitted == TRUE) ||
	(should_clear_page_received == TRUE)) {
      lpe_entry_t *tmplpe_CUR, *tmplpe_NEW;

      /* INTERRUPT LOOP DETECTED */
      if (should_clear_page_transmitted == TRUE)
	log(LOG_ERR,
	    "INTERRUPT LOOP DETECTED on Page Tranmitted, bus master cleared\n");
      if (should_clear_page_received == TRUE)
	log(LOG_ERR,
	    "INTERRUPT LOOP DETECTED on Page Received, bus master cleared\n");

      pciddc_status = hsl_conf_read((int) arg,
				    PCIDDC_STATUS);
      pci_status = hsl_conf_read((int) arg,
				 PCI_COMMAND_STATUS_REG);

      tmplpe_CUR = (lpe_entry_t *) hsl_conf_read((int) arg,
						 PCIDDC_LPE_CURRENT);
      tmplpe_NEW = (lpe_entry_t *) hsl_conf_read((int) arg,
						 PCIDDC_LPE_NEW);

      log(LOG_ERR, "loop: pciddc_status=0x%x\nloop: pci_status=0x%x\nloop: lpe_cur=0x%x lpe_new=0x%x\n",
	  (int) pciddc_status, (int) pci_status,
	  (((void *) tmplpe_CUR) -
	   ((void *) put_info[(int) arg].hsl_contig_space_phys)) / sizeof(lpe_entry_t),
	  (((void *) tmplpe_NEW) -
	   ((void *) put_info[(int) arg].hsl_contig_space_phys)) / sizeof(lpe_entry_t));

      if (should_clear_page_transmitted == TRUE)
	hsl_conf_write((int) arg,
		       PCIDDC_STATUS,
		       PCIDDC_STATUS_PAGE_TRANSMITTED);

      if (should_clear_page_received == TRUE)
	hsl_conf_write((int) arg,
		       PCIDDC_STATUS,
		       PCIDDC_STATUS_PAGE_RECEIVED);

      val = hsl_conf_read((int) arg,
			  PCI_COMMAND_STATUS_REG);
      hsl_conf_write((int) arg,
		     PCI_COMMAND_STATUS_REG,
		     val & ~PCI_COMMAND_MASTER_ENABLE);

      /* On bloque PUT pour regarder son etat */
      put_info[(int) arg].stop_put = TRUE;

      /* Enlever le Bus Master semble parfois etre inefficace -> on fait un soft reset... */
      /* Un panic (sans soft reset) semble etre aussi suffisant */
      /*     panic("interrupt loop suspected\n"); */

      /* Perform a soft reset */
      log(LOG_ERR,
	  "Performing a PCIDDC soft reset\n");
      hsl_conf_write((int) arg,
		     PCIDDC_COMMAND,
		     PCIDDC_SOFT_RESET);
    }
#endif

/*
log(LOG_DEBUG, "FIN-IRQ: current=0x%x,new=0x%x,softnew=0x%x\n",
put_info[(int) arg].lpe_current_virt,
put_info[(int) arg].lpe_new_virt,
put_info[(int) arg].lpe_softnew_virt);
*/

  return;
}

#endif

