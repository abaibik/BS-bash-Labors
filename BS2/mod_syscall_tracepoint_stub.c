/**
 * mod_syscall_tracepoint_stub.c - Stub für Laborversuch 2: Systemaufruf-Tracing
 *
 * Aufgabe: Implementieren Sie ein Linux-Kernelmodul, das alle Systemaufrufe
 * mit Hilfe von Kernel-Tracepoints zählt.
 *
 * Füllen Sie alle mit TODO markierten Stellen aus.
 * Hinweise und Fragen stehen als Kommentare direkt dabei.
 *
 * Kompilieren: make
 * Laden:       sudo insmod mod_syscall_tracepoint.ko
 * Testen:      cat /sys/kernel/debug/syscall_counter
 * Entladen:    sudo rmmod mod_syscall_tracepoint
 * Kernel-Log:  sudo dmesg | tail -20
 *
 * Autor:  <Ihr Name>
 * Lizenz: GPL
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/seq_file.h>
#include <linux/tracepoint.h>
#include <linux/tracepoint-defs.h>
#include <linux/spinlock.h>
#include <linux/version.h>
#include <linux/string.h> 
#include <linux/errno.h>  

/* Syscall-Namen (fertig bereitgestellt) */
#define WANT_SYSCALL_NAMES
#ifdef WANT_SYSCALL_NAMES
#  include "syscall_names.h"
#endif

/* -------------------------------------------------------------------------
 * TODO 1: Modul-Metadaten
 *
 * Tragen Sie Lizenz, Autor und Beschreibung ein.
 * Frage: Warum ist MODULE_LICENSE("GPL") für Tracepoints zwingend erforderlich?
 * ---------------------------------------------------------------------- */


/* -------------------------------------------------------------------------
 * Konstanten – passen Sie MAX_SYSCALLS bei Bedarf an
 * ---------------------------------------------------------------------- */
#define MAX_SYSCALLS   512

/* -------------------------------------------------------------------------
 * TODO 2: Globale Datenstrukturen
 *
 * Deklarieren Sie:
 *   a) Ein Array vom Typ `unsigned long`, das für jeden möglichen Syscall
 *      einen Zähler bereithält.
 *   b) Einen Zeiger auf struct tracepoint für den sys_enter-Tracepoint.
 *
 * ---------------------------------------------------------------------- */

/* TODO 2a: Zählerarray */

/* TODO 2b: Tracepoint-Zeiger */


/* -------------------------------------------------------------------------
 * TODO 3: Tracepoint-Probe-Funktion
 *
 * Implementieren Sie die Funktion probe_sys_enter(), die bei jedem
 * Systemaufruf vom Kernel aufgerufen wird.
 *
 * Signatur (muss exakt so lauten):
 *   static void probe_sys_enter(void *data, struct pt_regs *regs, long id)
 *
 * Parameter:
 *   data – optionale Benutzerdaten (hier nicht benötigt)
 *   regs – CPU-Register zum Zeitpunkt des Syscalls (hier nicht benötigt)
 *   id   – Nummer des Systemaufrufs (direkt vom Kernel übergeben!)
 *
 * Anforderungen:
 *   - Prüfen Sie, ob `id` im gültigen Bereich [0, MAX_SYSCALLS) liegt.
 *   - Sichern Sie den Zugriff auf das Zählerarray mit dem Spinlock.
 *   - Inkrementieren Sie den passenden Zähler.
 *
 * Frage: Was passiert, wenn Sie den Spinlock weglassen und zwei CPU-Kerne
 *        gleichzeitig denselben Syscall ausführen?
 * ---------------------------------------------------------------------- */
static void probe_sys_enter(void *data, struct pt_regs *regs, long id)
{
    /* TODO 3: Implementierung hier */
}


/* -------------------------------------------------------------------------
 * counter-Ausgabe (teilweise vorgegeben)
 * ---------------------------------------------------------------------- */

/**
 * syscall_counter_show - Gibt den Inhalt der counter aus
 *
 * TODO 4: Vervollständigen Sie die Ausgabeschleife.
 *
 * Anforderungen:
 *   - Iterieren Sie über alle MAX_SYSCALLS Einträge.
 *   - Geben Sie nur Einträge aus, deren Zähler > 0 ist.
 *   - Format: "%-4d %-8lu %s\n" mit (Index, Zählerstand, Syscall-Name)
 *   - Nutzen Sie syscall_names[i] für den Namen (oder "unbekannt" als Fallback).
 *
 * Frage: Warum verwenden wir seq_printf statt printk?
 */
static int syscall_counter_show(struct seq_file *m, void *v)
{
    /* TODO 4a: Kopfzeile ausgeben mittels seq_printf */

    /* TODO 4b: Schleife und Ausgabe der Zähler */

    return 0;
}

/* open()-Handler – nicht verändern */
static int syscall_counter_open(struct inode *inode, struct file *file)
{
    return single_open(file, syscall_counter_show, NULL);
}


/* -------------------------------------------------------------------------
 * TODO 5: Tracepoint suchen
 *
 * Implementieren Sie zwei Funktionen:
 *
 * a) lookup_sys_enter_tp(struct tracepoint *tp, void *priv)
 *    Callback für for_each_kernel_tracepoint().
 *    Speichert den Zeiger auf sys_enter in *priv, wenn tp->name == "sys_enter".
 *
 * b) find_sys_enter_tp(struct tracepoint **tp_ptr)
 *    Nutzt for_each_kernel_tracepoint() mit dem obigen Callback.
 *    Gibt 0 bei Erfolg zurück, -ENOENT wenn nicht gefunden.
 *
 * Frage: Warum iterieren wir zur Laufzeit über alle Tracepoints statt den
 *        Zeiger direkt per Symbol-Name zu ermitteln?
 * ---------------------------------------------------------------------- */


/* TODO 5a: sys_enter-Tracepoint suchen */
static void lookup_sys_enter_tp(struct tracepoint *tp, void *priv)
    {
        struct tracepoint **tp_ptr = priv;

        // Prüfen, ob der Tracepoint "sys_enter" heißt
        if (strcmp(tp->name, "sys_enter") == 0) {
            *tp_ptr = tp;
        }
    }

/* TODO 5b: Suche starten und Ergebnis prüfen */
static int find_sys_enter_tp(struct tracepoint **tp_ptr)
    {
        *tp_ptr = NULL;

        // Alle Kernel-Tracepoints durchsuchen
        for_each_kernel_tracepoint(lookup_sys_enter_tp, tp_ptr);

        // Fehler zurückgeben, falls nicht gefunden
        if (*tp_ptr == NULL) {
            return -ENOENT;
        }

        return 0;
    }



/* -------------------------------------------------------------------------
 * TODO 6: Tracepoint registrieren und deregistrieren
 *
 * a) register_tp():
 *    - Ruft find_sys_enter_tp() auf.
 *    - Registriert probe_sys_enter mit tracepoint_probe_register() bzw.
 *      tracepoint_probe_register_prio() (ab Kernel 5.11).
 *    - Gibt 0 bei Erfolg zurück.
 *
 * b) unregister_tp():
 *    - Ruft tracepoint_probe_unregister() auf.
 *    - Ruft danach ZWINGEND tracepoint_synchronize_unregister() auf.
 *
 * Frage: Was kann passieren, wenn tracepoint_synchronize_unregister()
 *        fehlt und das Modul entladen wird, während ein CPU-Kern noch
 *        in probe_sys_enter() aktiv ist?
 * ---------------------------------------------------------------------- */

/* TODO 6a: register_tp */

/* TODO 6b: unregister_tp */


/* -------------------------------------------------------------------------
 * TODO 7: Modul-Init und -Exit
 *
 * mod_syscall_tracepoint_init():
 *   1. Zählerarray mit memset() auf 0 initialisieren.
 *   3. Tracepoint registrieren (register_tp()).
 *   4. Bei Fehler aufräumen und negativen Fehlercode zurückgeben.
 *   5. Erfolgsmeldung mit printk(KERN_INFO ...) ausgeben.
 *
 * mod_syscall_tracepoint_exit():
 *   1. Tracepoint deregistrieren (unregister_tp()).
 *   3. Meldung mit printk(KERN_INFO ...) ausgeben.
 *
 * ---------------------------------------------------------------------- */

 /* TODO 7a: Init-Implementierung */

 /* TODO 7b: Exit-Implementierung */

module_init(mod_syscall_tracepoint_init);
module_exit(mod_syscall_tracepoint_exit);