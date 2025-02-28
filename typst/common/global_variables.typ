#let atomic-boxes = state("atomic-boxes", (:)) // stockage des boîtes atomiques
#let copy-counter = counter("copy-counter") // compteur de copies
#let page-state = state("page-state", (:)) // stockage des états de la page 
#let generating-content = state("generating-content", true) // indicateur de génération de contenu