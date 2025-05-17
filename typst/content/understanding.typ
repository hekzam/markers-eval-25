#import "../components/container.typ": container
#import "../components/id-consent-box.typ": exo-pts, id-consent-box
#import "@preview/codly:1.0.0": *

#let cc-id = 9
#set par(justify: true)
#set raw(theme: "theme.tmTheme")

#import "@preview/sourcerer:0.2.1": code

#let dim = (
  checkbox-width: 3mm,
  checkbox-height: 3mm,
  fillbox-height: 8mm,
  digitbox-width: 6mm,
  digitbox-height: 8mm,
)

#let understanding-1 = [
  #set align(left+top)
  #id-consent-box(cc-id, ..dim)

  == Topic 1: Job Reordering in EASY Backfilling
  What do the following acronyms stand for?
  #grid(
    columns: 2,
    align: horizon + right,
    column-gutter: 3mm,
    [FCFS], container("1-meaning-fcfs", 10cm, 5mm),
    [LPF], container("1-meaning-lpf", 10cm, 5mm),
    [SQF], container("1-meaning-sqf", 10cm, 5mm),
  )

  At what point in the EASY Backfilling algorithm do the authors propose to add queue reordering?
  #container("1-queue-reordering-when", 100%, 3cm)


  == Topic 3: Backfilling under an Energy Budget
  Is there a difference between an energy budget and a power budget? If so, what are they?
  #container("3-diff-power-energy-budget", 100%, 3cm)

  What is a powercap ?
  #container("3-powercap", 100%, 3cm)
]

#let understanding-2 = [
  #set align(left+top)

  == Topic 2: Conservative Backfilling
  Briefly explain the differences between EASY Backfilling and Conservative Backfilling.
  #container("2-diff-easy-conservative", 100%, 4cm)

  What does schedule compression mean for Conservative Backfilling?
  #container("2-schedule-compression", 100%, 4cm)

  == Topic 4: Contiguity and Locality Constraints in Backfilling
  What's the difference between contiguity and locality?
  #container("4-diff-contiguity-locality", 100%, 4cm)

  What's the difference between "best effort" and "forced" variants of the proposed algorithms?
  #container("4-diff-besteffort-forced", 100%, 4cm)
]
