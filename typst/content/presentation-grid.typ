#import "../components/container.typ": container
#import "../components/id-consent-box.typ": exo-pts, id-consent-box
#import "@preview/codly:1.0.0": *

#let cc-id = 8
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

#let b(grp, q, scale) = box(container(("pscale", grp, q, scale).join("-"), dim.checkbox-width, dim.checkbox-height))
#let l(grp, q) = ("excellent", "good", "satisfactory", "needs_improvement", "unsatisfactory").map(val => b(grp, q, val))

#let top-grid(grp) = grid(
  columns: 6,
  column-gutter: 5mm,
  row-gutter: 1em,
  align: (x,y) => {
    if x == 0 { right + horizon }
    else { center + horizon}
  },
  grid.header(repeat: false, [], [Excellent], [Good], [Satisfactory], [Needs improvement], [Unsatisfactory]),
  [Technical quality of the work produced], ..l(grp, "technical_quality"),
  [Scientific quality of the approach], ..l(grp, "scientific_approach"),
  [Oral communication skills], ..l(grp, "oral_communication"),
  [Quality of slides], ..l(grp, "slides_quality"),
  [Quality of answers to questions], ..l(grp, "questions"),
)

#let presentation-grid-1 = [
  #set align(left+top)
  #id-consent-box(cc-id, ..dim)

  = Instructions
  Indicate who you are at the top of this page.

  Evaluate the work of each group *including your own* by filling in the evaluation grid overleaf (on page 2 / 2).
]

#let presentation-grid-2 = [
  #set align(left+top)
  
  == Topic 1: Job Reordering in EASY Backfilling
  #top-grid("1_easy")
  #line(length: 100%)

  == Topic 2: Conservative Backfilling
  #top-grid("2_conservative")
  #line(length: 100%)

  == Topic 3: Backfilling under an Energy Budget
  #top-grid("3_energy_budget")
  #line(length: 100%)

  == Topic 4: Contiguity and Locality Constraints in Backfilling
  #top-grid("4_contiguity_locality")
]