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
#include <linux/init.h>
#include <linux/string.h>
#include <linux/errno.h>  
#include <linux/proc_fs.h>

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
static unsigned long syscall_counts[MAX_SYSCALLS];

/* TODO 2b: Tracepoint-Zeiger */
static struct tracepoint *sys_enter_tp;


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


/* /proc/syscall_counter - Schnittstelle */

// Zeiger auf den /proc-Eintrag
static struct proc_dir_entry *syscall_proc_entry;

// Funktionen zum Lesen der Datei
static const struct proc_ops syscall_counter_proc_ops = {
    .proc_open    = syscall_counter_open,
    .proc_read    = seq_read,
    .proc_lseek   = seq_lseek,
    .proc_release = single_release,
};

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


/* TODO 6a: Tracepoint registrieren */
static int register_tp(void)
    {
        int ret;

        // sys_enter-Tracepoint suchen
        ret = find_sys_enter_tp(&sys_enter_tp);
        if (ret != 0) {
        sys_enter_tp = NULL;
        return ret;
}

        // Probe registrieren
        ret = tracepoint_probe_register(
            sys_enter_tp,
            (void *)probe_sys_enter,
            NULL
        );

        if (ret != 0)
            return ret;

        return 0;
    }

/* TODO 6b: Tracepoint deregistrieren */
static void unregister_tp(void)
    {
        if (sys_enter_tp == NULL)
            return;

        // Probe deregistrieren
        tracepoint_probe_unregister(
            sys_enter_tp,
            (void *)probe_sys_enter,
            NULL
        );

        // Auf laufende Probe-Aufrufe warten
        tracepoint_synchronize_unregister();

        sys_enter_tp = NULL;
    }



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



/* TODO 7a: Modul initialisieren */
static int __init mod_syscall_tracepoint_init(void)
{
    int ret;

    // Alle Zähler auf 0 setzen
    memset(syscall_counts, 0, sizeof(syscall_counts));

    // Tracepoint registrieren
    ret = register_tp();

    // Bei Fehler Initialisierung abbrechen
    if (ret != 0) {
        printk(KERN_ERR "syscall_tracepoint: Registrierung fehlgeschlagen: %d\n", ret);
        return ret;
    }

    // /proc/syscall_counter erstellen
    syscall_proc_entry = proc_create(
        "syscall_counter",
        0444,
        NULL,
        &syscall_counter_proc_ops
    );

    // Bei Fehler Tracepoint wieder deregistrieren
    if (syscall_proc_entry == NULL) {
        printk(KERN_ERR "syscall_tracepoint: /proc konnte nicht erstellt werden\n");
        unregister_tp();
        return -ENOMEM;
    }

    // Erfolgsmeldung ausgeben
    printk(KERN_INFO "syscall_tracepoint: Modul geladen\n");

    return 0;
}

/* TODO 7b: Modul entladen */
static void __exit mod_syscall_tracepoint_exit(void)
{
    // /proc-Eintrag entfernen
    proc_remove(syscall_proc_entry);

    // Tracepoint deregistrieren
    unregister_tp();

    // Erfolgsmeldung ausgeben
    printk(KERN_INFO "syscall_tracepoint: Modul entladen\n");
}


module_init(mod_syscall_tracepoint_init);
module_exit(mod_syscall_tracepoint_exit);