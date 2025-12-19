Warning:  When building it is best right now to stick right with using Qt 6.7.1  Qt 6.7.2 has a bug in reparenting QWidgets which stops some QCombBoxes from operating.  Qt 6.7.3 throws an ASSERT in QDate::toSttring in debug mode and so should also be avoided.  This second problem is also present in Qt 6.8.0 so far, prior to release. 

This application is a work-in-progress, an exploration of geometric art.  It started by my being inspired by the stunningly beautiful creations to be seen in Andalucia.  This artform is often described as Islamic Art and it is true that its primary exemplars are to be found in locations where Islam is dominant in the culture.  However it seems to me that this art emerged in a culture where Islam, Judaism, and Christianity co-existed.


In terms of the software, what we have here a program called **Tiled Pattern Maker**  which is a hybrid.  I started to write my own program from scratch, but ran into difficulties creating some designs.  I then encountered the application Taprats (seemingly a semordnilap of Star Pat).  Taprats was created by Craig Kaplan and later extended by Pierre Baillargeon.  I obtained taprats source code  from [https://sourceforge.net/projects/taprats/](https://sourceforge.net/projects/taprats/)  using hg to clone the repository.  Taprats is written in Java to which I have an irrational aversion.  So I have ported taprats to C++ using the Qt application framework.  To build and run this project you need to download Qt and use QtCreator to build the app.  It runs on Linux, MacOS, and Windows.  


In porting the software from taprats I have really dissected it for my own edification to see how Kaplan and Baillargeon have implemented the seminal works of Hankin, A.J. Lee, and others.  What I am exposing as version 1.0 contains basically the port of taprats plus some extensions of my own, a revamped GUI (not necessarily better, just different) and many debug statements sprinkled through the code.  I see this version 1.0 as a platform for future work by myself.


In terms of copyright, this software is freely given without warranty.  In keeping with what Craig Kaplan did, it is marked with being under the terms of GNU GPL.  

Download pre-built win10 executable [here](https://github.com/ChortleMortal/TiledPatternMaker/releases)





