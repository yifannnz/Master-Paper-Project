% ---------------------------------------------------------------------
% EG author guidelines plus sample file for EG publication using LaTeX2e input
% D.Fellner, v2.04, Dec 14, 2023
\usepackage{graphicx}
\usepackage{makecell}
\usepackage{pifont}
\usepackage{threeparttable}
\usepackage{cuted}
\usepackage{color}
\usepackage{caption}
\usepackage{amsmath}
\usepackage{subfigure}
\usepackage{multirow}
\usepackage[ruled]{algorithm2e}

\title[Multiphase Viscoelastic Non-Newtonian Fluid Simulation]%
      {Multiphase Viscoelastic Non-Newtonian Fluid Simulation}

% for anonymous conference submission please enter your SUBMISSION ID
% instead of the author's name (and leave the affiliation blank) !!
% for final version: please provide your *own* ORCID in the brackets following \orcid; see https://orcid.org/ for more details.
\author{paper1038}
%        \\
% For Computer Graphics Forum: Please use the abbreviation of your first name.
%{\parbox{\textwidth}{\centering $^1$TU Darmstadt \& Fraunhofer IGD, Germany\\
%         $^2$Graz University of Technology, Institute of Computer Graphics and Knowledge Visualization, Austria
%        $^2$ Another Department to illustrate the use in papers from authors
%             with different affiliations
%       }
%}
%}


% ------------------------------------------------------------------------

% if the Editors-in-Chief have given you the data, you may uncomment
% the following five lines and insert it here
%
% \volume{36}   % the volume in which the issue will be published;
% \issue{1}     % the issue number of the publication
% \pStartPage{1}      % set starting page





%-------------------------------------------------------------------------
\begin{document}

% uncomment for using teaser
% \teaser{
%  \includegraphics[width=0.9\linewidth]{eg_new}
%  \centering
%   \caption{New EG Logo}
% \label{fig:teaser}
%}


\maketitle




%-------------------------------------------------------------------------
\begin{abstract}
  We propose an SPH-based method for simulating viscoelastic non-Newtonian fluids within a multiphase framework. For this, we use mixture models to handle component transport and conformation tensor methods to handle the fluid's viscoelastic stresses. In addition, we consider a bonding effects network to handle the impact of microscopic chemical bonds on phase transport. Our method supports the simulation of both steady-state viscoelastic fluids and discontinuous shear behavior. Compared to previous work on single-phase viscous non-Newtonian fluids, our method can capture more complex behavior, including material mixing processes that generate non-Newtonian fluids. We adopt a uniform set of variables to describe shear thinning, shear thickening, and ordinary Newtonian fluids while automatically calculating local rheology in inhomogeneous solutions. In addition, our method can simulate large viscosity ranges under explicit integration schemes, which typically requires implicit viscosity solvers under earlier single-phase frameworks.
%-------------------------------------------------------------------------
%  ACM CCS 1998
%  (see https://www.acm.org/publications/computing-classification-system/1998)
% \begin{classification} % according to https://www.acm.org/publications/computing-classification-system/1998
% \CCScat{Computer Graphics}{I.3.3}{Picture/Image Generation}{Line and curve generation}
% \end{classification}
%-------------------------------------------------------------------------
%  ACM CCS 2012
%ALEX: commented out: (see https://www.acm.org/publications/class-2012)
%The tool at \url{http://dl.acm.org/ccs.cfm} can be used to generate
% CCS codes.
%Example:

\begin{CCSXML}
<ccs2012>
   <concept>
       <concept_id>10010147.10010371.10010352.10010379</concept_id>
       <concept_desc>Computing methodologies~Physical simulation</concept_desc>
       <concept_significance>500</concept_significance>
       </concept>
 </ccs2012>
\end{CCSXML}

\ccsdesc[500]{Computing methodologies~Physical simulation}



% key words

\keywords{fluid simulation, mixture models, Viscoelastic Non-Newtonian fluids}

\printccsdesc   
\end{abstract}  



%-------------------------------------------------------------------------
\section{Introduction}
\label{sec:intro}
%
Fluid simulation has always been a research hotspot in computer graphics\,\cite{Wang2024}. Many techniques have been proposed, including Newtonian fluid simulation, detail enhancement such as turbulence\,\cite{feng2022impulse}, and multiphase fluid simulation\,\cite{xu2023implicitly}. In Newtonian fluid simulations, previous work has covered surface tension\,\cite{jeske2023implicit}, pressure solvers\,\cite{bender2015divergence}, and viscous solvers \cite{takahashi2015implicit,weiler2018physically,liu2021adapted,bender2016divergence} \cite{peer2015implicit}; as well as numerical accuracy improvements for different calculation methods\,\cite{hirasawa2021flux}. In multiphase fluid simulations, simulated phenomena include bubbles \cite{hong2003animation,ihmsen2011animation,takahashi2003realistic}, melting\,\cite{muller2004point,chang2009particle,fujisawa2007animation}, and crystallization\,\cite{kim2003visual,kim2004hybrid,kim2006modeling}, among several others.

Newtonian fluids can be considered time-invariant with constant viscosity under certain conditions. In contrast, non-Newtonian fluids show a nonlinear relationship between viscosity and shear strain rate. Non-Newtonian fluids are actually more common in industrial production and in nature, \emph{e.g.}, blood, chocolate, paint, and glass melts. At present, single-phase techniques can simulate simple non-Newtonian fluids\,\cite{liu2021adapted,andrade2014sph} by computing the fluid's rheology by adding macroscopic viscosity models such as the Cross model~\cite{cross65} to existing fluid simulators. For example, when configuring a specific fluid, such as the shear thinning type, the parameters of the Cross model are fixed in one single simulation. As the local rheological properties of a mixture may be different due to the solute concentration, such methods cannot simulate complex mixtures. Rather, when substances are mixed to form a non-Newtonian fluid (and since almost all non-Newtonian fluids are mixtures), simulation mechanisms with a multiphase framework are preferred. 

This paper proposes a non-Newtonian fluid simulation method based on multiphase flow. Our method focuses on two-phase flow simulation and uses the conformation tensor method of non-Newtonian fluid dynamics to calculate the fluid's viscoelastic stress, and designs a non-Newtonian fluid mechanism based on the mixture model, which can simulate shear thinning fluids, shear thickening fluids, and Newtonian fluids in a single framework. Our method can process single-phase non-Newtonian fluids, deal with dynamic mixing scenes, and compute the local rheological properties of the fluid separately. Our method has simple-to-set parameters and can be computed efficiently. Separately, in previous multiphase fluid models, interphase momentum exchange depends on suitably setting various constants (\emph{e.g.} $k_d$ in\,\cite{jiang2021dynamic}; and $C_d$ in\,\cite{xu2023implicitly}). Such settings imply a \emph{constant} degree of the mixture effect on phase motion for an entire simulation -- which does not hold for systems with significant combining reactions. To the best of our knowledge, ours is the first action model to effectively alleviate the limitations of previous multiphase fluid models where interphase momentum exchange is constrained by constant settings. Our model uniquely calculates the effect of concentration on phase transfer by simulating the dynamic network of chemical bonds formed in the solution, thus providing a more accurate representation of the mixing dynamics in non-Newtonian fluids.

\section{Related Work}
\label{sec:background}
%
Much previous work on fluid simulation focuses on Newtonian fluids. Simulating non-Newtonian fluids is mainly achieved by viscosity solvers combined with simple non-Newtonian fluid viscosity models. In such schemes, a core problem is how to solve the viscous term in the Navier-Stokes equations and the viscosity of non-Newtonian fluids.

\smallskip
\noindent\textbf{Viscosity computation:} Currently, there are three main methods to solve the viscosity term in the SPH method: artificial viscosity (XSPH), viscosity Laplacian-based methods, and strain rate tensor-based methods~\cite{koschier2022survey}. Schechter \emph{et al.}\,\cite{schechter2012ghost} used XSPH to compute viscosity, which updates the velocity field by controlling the velocity difference with artificial parameters. However, such artificial parameters lead to the problem that phenomena such as curling and coiling cannot be correctly simulated. Many methods exist that handle the Laplacian of viscosity. A simple approach is to discretize the Laplacian by standard SPH. While easy to compute, this method is highly sensitive to the lack of particles at the boundary, and it cannot guarantee the sign correctness of the second derivative of the Gaussian-like kernel function. To avoid second derivatives, Takahashi \emph{et al.}~\cite{takahashi2015implicit} solved the first derivative twice, but the computational overhead increased. Brokshaw \emph{et al.}~\cite{brookshaw1985method} used an SPH discretization combined with finite differences to process the Laplacian and conserve the linear and angular momenta. This method was later adopted 
by subsequent schemes~\cite{weiler2018physically,liu2021adapted}. 

\smallskip
\noindent\textbf{Improving quality and speed:} To achieve faster and stabler simulation of highly viscous fluids, many methods proposed implicit viscosity solvers based on strain rate tensors. Takahashi \emph{et al.}~\cite{takahashi2015implicit} used implicit time integration in a reverse Euler framework based on the original strain rate tensor representation. Though this scheme allowed for larger time steps, it led to pseudo-realistic artifacts in some scenes due to missing particles at the boundary. Bender \emph{et al.}~\cite{bender2016divergence} proposed a velocity constraint function with the strain rate tensor to correct the velocity field, which is, however, expensive to compute.
Peer \emph{et al.}~\cite{peer2015implicit} decomposed the velocity gradient tensor into three parts, with an artificial regulator imposed on the shear rate tensor; SPH discretization and Taylor expansion were used to numerically approximate the velocity field. However, this scheme cannot smoothly reconstruct the velocity field for  low-viscous fluids. Also, this velocity field reconstruction method produces artifacts due to the abovementioned missing particle issue. The Laplacian-based optimization scheme in~\cite{weiler2018physically} addressed the issue of boundary artifacts of the above methods. In addition, some acceleration schemes also consider the use of efficient data structures and multi-grid strategies, Goldade \emph{et al.}~\cite{goldade2019adaptive} proposed an implicit octree finite difference discretization with adaptive staggered grids, significantly accelerating the viscous solution of the free surface. Shao \emph{et al.}~\cite{shao2022fast} presented an Unsmoothed Aggregation Algebraic MultiGrid (UAAMG) method with a multi-color Gauss-Seidel smoother to speed up sparse matrix operations.

To further improve visual realism in large viscous fluid simulations,  researchers have studied the interference problem between incompressibility and viscosity solutions. Liu \emph{et al.}~\cite{liu2021adapted} first implemented a convergent sticky-incompressibility iterative algorithm in the SPH method by using the SIMPLE algorithm in an Eulerian setting, which reduces the interference between solvers; however, the iterative nature of this method can yield  high computational overhead. Based on such SPH solvers, some studies further extended simulations to non-Newtonian fluids. Andrade \emph{et al.}~\cite{andrade2014sph} combined the symmetric form of Laplacian discretization with a Cross model~\cite{cross65} to simulate simple shear-thinning non-Newtonian fluids. Ozgen \emph{et al.}~\cite{ozgen2019sph} introduced an efficient history-based stiffness term and used the fractional calculus to describe shear thickening fluids in the SPH framework. Liu \emph{et al.}~\cite{liu2021adapted} used the Cross model~\cite{cross65} to describe simple shear thinning and shear thickening scenarios.

\smallskip
\noindent\textbf{Refined frameworks:}
Some viscous solvers achieve good visual results using more refined frameworks. The codimension method~\cite{zhu2015codimensional} used the Carreau model~\cite{carreau1972rheological} to describe shear thinning and shear thickening fluids, achieving a good thin film effect; yet, this method involves complex calculations. Barreiro \emph{et al.}~\cite{barreiro2017conformation} were the first to implement real-time simulation of viscoelastic Newtonian fluids using polymer conformation methods combined with Position Based Dynamics (PBD); however, they did not consider non-Newtonian fluids. Goldade \emph{et al.}~\cite{goldade2019adaptive} used an octree adaptive variational finite difference method to optimize the viscosity solution problem, which greatly increased speed. Fei \emph{et al.}~\cite{fei2019multi} developed a multi-scale framework for coupling wire and shear-related fluids, where the elastoplastic fluid was modeled by the Herscher-Barkley model. Panuelos \emph{et al.}~\cite{panuelos2023polystokes} used the functional method to solve the unsteady Navier-Stokes equations to simulate highly viscoelastic fluids. The advantage of this method is that it preserves the coupling between the viscosity and pressure solutions and improves numerical accuracy. Su \emph{et al.}~\cite{su2021unified} used the MPM method to construct a second-order accurate viscosity solver that achieves diverse single-fluid effects while maintaining good numerical accuracy. By integrating previously simulated models of different materials into a unified particle framework, Li \emph{et al.}~\cite{li2023unified} created a comprehensive engine able to simulate a wide variety of non-Newtonian fluids.

\smallskip
\noindent\textbf{Multiphase simulations:}
The above mentioned methods use a \emph{single-phase} framework and cannot deal with multivariate dynamic scenarios, such as material mixing and precipitation phenomena. For such cases, multiphase simulations are needed. Multiphase simulation frameworks aim to address this case, which is consistent with the fact that fluids almost always exist as mixtures. Separately, single-phase models can only simulate fluids with fixed properties; and non-Newtonian viscosity models can only realize simple non-Newtonian fluid simulations. The formation of non-Newtonian fluids after material mixing and the simulation of the mixing process can only be achieved using a multiphase framework. 

The mixture model, introduced by Ren \emph{et al.}~\cite{ren2014multiple} to graphics, is a simple to compute and easy to extend multiphase model.  Jiang \emph{et al.}~\cite{jiang2021dynamic} further reformulated the computation of the hybrid model, abandoned the local equilibrium assumption, and enhanced simulation efficiency and stability, allowing the simulation of more dynamic scenes. Xu \emph{et al.}~\cite{xu2023implicitly} proposed an implicit mixture model and used a volume-incompressible method to enforce fluid incompressibility, leading to a more stable phase transport and a highly improved 
numerical stability.

The phenomena investigated by current multiphase fluid simulations are very broad, including bubble simulation\,\cite{hong2003animation,ihmsen2011animation,takahashi2003realistic}, melting~\cite{muller2004point,chang2009particle,fujisawa2007animation}, and crystal formation~\cite{kim2003visual,kim2004hybrid,kim2006modeling}. However, in the field of computer graphics, there is no current approach that attempts to model non-Newtonian fluids with multiphase models. We present a method that enables efficient computation of dynamically non-Newtonian fluid scenarios under a multiphase model.

\section{Implicit Mixture Model with SPH}
\label{sec:baseline}

The implicit mixture model is a modification based on mixture model theory, which is usually computed numerically using the SPH method in graphics. We next review the fundamentals of SPH (Sec.~\ref{sec:sph}) and outline the implicit mixture model (Sec.~\ref{sec:mixturemodel}).

\subsection{SPH method}
\label{sec:sph}

The SPH (Smoothed Particle Hydrodynamics) method is a spatial discretization technique. It uses a set of kernel functions to represent the weights of the space and reconstructs the physical field of the fluid based on the weights. The kernel function is usually Gaussian-like; practical implementations use cubic spline functions
\begin{equation}
    W(\mathbf{r}, h) = \Lambda
    \begin{cases}
        6(l^3-l^2)+1, & 0 \leq l \leq 0.5 \\
        2(1-l)^3, & 0.5 < l \leq 1 \\
        0, & otherwise
    \end{cases}
\end{equation}
where $l = \frac{\|\mathbf{r}\|}{h}$, $h$ is the support radius of the kernel, and $\Phi$ depends on the spatial dimension, 1D$: \Lambda=\frac{4}{3h}$, 2D$: \Lambda=\frac{40}{7\pi h^2}$, 3D$: \Lambda=\frac{8}{\pi h^3}$.

For a field $\mathbf{A}$, SPH uses neighborhood information to compute the value, gradient, and divergence of the field, as 
%
\begin{equation}
    \begin{aligned} 
        \mathbf{A}_i &= \sum _{j \in N(i)} \frac{m_j}{\rho _j}\mathbf{A}_j W^{ij}, \\
        \nabla \mathbf{A}_i &= \sum _{j \in N(i)} \frac{m_j}{\rho _j}\mathbf{A}_j \otimes \nabla W^{ij}, \\
        \nabla \cdot \mathbf{A}_i &= \sum _{j \in N(i)} \frac{m_j}{\rho _j}\mathbf{A}_j\cdot \nabla W^{ij},\\
        \nabla ^2 \mathbf{A}_i &= \sum _{j \in N(i)} \frac{m_j}{\rho _j}\mathbf{A}_j \nabla ^2 W^{ij}, \\
    \end{aligned}
\end{equation}
where $\mathbf{a} \otimes \mathbf{b} = \mathbf{a}\mathbf{b}^T$, $i$ is the index of current particle, $j$ is the neighbor index, $N(i)$ are the neighbors of particle $i$, and $W^{ij} = W(\mathbf{x}_i-\mathbf{x}_j, h)$.
For brevity, we next use $\sum_j$ to denote $\sum _{j \in N(i)}$.

\subsection{Implicit Mixture Model}
\label{sec:mixturemodel}
%
The \emph{mixture model}~\cite{Mikko96} is a well-known technique to simulate multiphase fluids. It uses the volume fraction (Fig.~\ref{fig:volume_fraction}) to represent the concentration ratio of each phase in a multiphase particle; computes the dynamic parameters of each phase through the multiphase fluid dynamics; and finally reconstructs the velocity field of the mixed particle. We next detail the mixture model's main terms.

\begin{figure}[htb]
\centering
\begin{overpic}[width=\columnwidth]{SCA_PICS/ims.pdf}
\put(3,52){\scriptsize \textbf{Volume Fraction Scheme}}
\put(45,52){\scriptsize \textbf{Momentum Source in Implicit-Mixture-Model}}
\put(3,24){\scriptsize \textbf{Momentum Contribution}}
\put(9,48){\tiny $\mathbf{v}_2^d$}
\put(22,46){\tiny $\mathbf{v}_1^d$}
\put(2,48){\tiny $\mathbf{v}_2$}
\put(15,45){\tiny $\bar{\mathbf{v}}$}
\put(30,38){\tiny $\mathbf{v}_1$}
\put(10,37){\tiny $\alpha_2^i$}
\put(45,15){\tiny $\alpha_2^i$}
\put(20,32){\tiny $\alpha_1^i$}
\put(50,8){\tiny $\alpha_1^i$}
\put(18,17){\tiny $\mathbf{M}_k^\mathrm{p}$}
\put(22,10){\tiny $\mathbf{M}_k^\mathrm{I}$}
\put(17,4){\tiny $\mathbf{M}_k^\mathrm{g}$}
\put(40,21){\tiny $\mathbf{v}_2'$}
\put(60,2){\tiny $\mathbf{v}_1'$}
\put(38,4){\tiny $\bar{\mathbf{v}}$}
\put(30,19){\scriptsize change}
\put(63,19){\scriptsize reconstruct}
\put(30,16){\scriptsize phase-vel}
\put(63,16){\scriptsize mix-vel}
\put(35,39){\footnotesize $\sum_k \alpha_k^i=1$}
\put(50,47){\footnotesize $\rho_k\frac{D\mathbf{v}_k}{Dt}  = \mathbf{M}_k = \mathbf{M}^\mathrm{p}_k + \mathbf{M}^\mathrm{nonp}_k + \mathbf{M}^\mathrm{I}_k$}
\put(50,42){\scriptsize $\mathbf{M}^\mathrm{p}_k$: VFSPH Solver with momentum mapping}
\put(50,37){\scriptsize $\mathbf{M}^\mathrm{nonp}_k$: gravity, …}
\put(50,32){\scriptsize $\mathbf{M}^\mathrm{I}_k$: analytic interphase momentum $C_d\mathbf{M}_k^\mathrm{I'}$ }
\put(78,14){\footnotesize $\bar{\mathbf{v}}'=\sum_k\alpha_k\mathbf{v}_k'$}
\end{overpic}
\caption{The volume fraction scheme and momentum sources in the implicit mixture model~\cite{xu2023implicitly}.}
\label{fig:volume_fraction}
\end{figure}

% %
% A mixed particle $i$ with phases $k$ contains the volume fractions $\alpha^i_k$ of each phase which sum up to 1, that is
% %
% \begin{equation}
% \alpha^i = \sum_k \alpha^i_k = 1.
% \end{equation}
% %
% The mixture model uses the drift velocity $\mathbf{v}_{mk}$ to measure the shift of the phase velocity $\mathbf{v}_k$ relative to the mixture velocity $\mathbf{v}_m$ as
% %
% \begin{equation}
% \mathbf{v}_{mk} = \mathbf{v}_k - \mathbf{v}_m.
% \label{eq:2}
% \end{equation}
% %
% Next, $\mathbf{v}_{mk}$ is used to handle phase transport. In the above, $\mathbf{v}_m$ field is computed by weighting the phase velocity according to the volume fraction
% %
% \begin{equation}
% \mathbf{v}_m = \sum _ k \alpha _k \mathbf{v}_k
% \label{eq:3}
% \end{equation}
% %
% \alex{Please see comments in Overleaf, there are still notation problems. But I \emph{think} I know where they come from and how to solve them: $\alpha_k$ is a scalar \emph{field}, taking the values $\alpha^i_k$ at the particles $i$, right? If so, please confirm this in the text -- I can easily fix the sentence above Eqn 1 to make this all consistent. [This indeed needs to be fixed; see Jiri's email.]}
% %
% Finally, the particle rest density $\rho_m$ is computed as
% \begin{equation}
% \rho_m = \sum _k \alpha_k\rho^0_k
% \end{equation}
% %
% where $\rho^0_k$ is the rest density of phase $k$.

% The recent implicit mixture model in~\cite{xu2023implicitly} is designed to improve the numerical stability of mixture models. It uses a constant rest volume of particles and adopts the profile $p_k = p_m$ for mixture fluids. The basic implicit model has five phase-level momentum sources: pressure $\mathbf{M}^p_k$, viscosity $\mathbf{M}^v_k$, gravity $\mathbf{M}^g_k$, advection $\mathbf{M}^a_k$, and interphase $\mathbf{M}^I_k$; the corresponding velocity fields and mixture momenta are denoted accordingly, \emph{e.g.}, $\mathbf{v}_k^{v}$ and $\mathbf{M}_m^v$ for $\mathbf{M}^v_k$. In detail: $\mathbf{M}^a_k$ is caused by the phase drift velocity;  $\mathbf{M}^I_k$ obeys a strict force balance model $\sum_k \mathbf{M}^I_k=0$; according to Eqn.~\ref{eq:2}, we get $\mathbf{M}_m = \mathbf{M}^*_k + \mathbf{M}^d_k$; finally, the mixture momentum sources can be obtained by summing up all momentum sources of all phases as
% %
% \begin{equation}
% \alpha_k\rho_k\frac{D\mathbf{v}_k}{Dt} = \mathbf{M}^p_k + \mathbf{M}^v_k + \mathbf{M}^g_k + \mathbf{M}^a_k + \mathbf{M}^I_k
% \label{eq:5}
% \end{equation}
% %
% \begin{equation}
% \rho_m\frac{D\mathbf{v}_m}{Dt} = \mathbf{M}^p_m + \mathbf{M}^v_m + \mathbf{M}^g_m + \mathbf{M}^a_m + \mathbf{M}^d_m
% \label{eq:6}
% \end{equation}
% %
% From Eqns.~\ref{eq:2}, \ref{eq:5}, and \ref{eq:6}, we get the change rate of phase drift velocity as
% \begin{equation}
% \frac{D\mathbf{v}_{mk}}{Dt} = \frac{\mathbf{M}^*_k}{\alpha_k\rho_k} - \frac{\mathbf{M}^*_m}{\rho_m}
% \end{equation}
% where $*$ denotes all relevant momentum sources ($p,v,g,a,I$).

% The relationship between $\mathbf{M}^I_k$ and $\mathbf{M}^d_k$ can be deduced from the no-drift and free-drift situations to finally get  $\mathbf{M}^I_k=C_d\mathbf{M}_k^{I^{'}}$, where $\mathbf{M}_k^{I^{'}}$ is number interphase momentum and $C_d \in [0, 1]$ is a regulatory factor. $C_d=0$ recovers free-drift which means interphase momentum is zero and $C_d=1$ recover no-drift which means phase drift velocity is zero.

% Computing the momentum sources $\mathbf{v}^{v}_{k}$ and $\mathbf{v}^p_k$ uses $C_d$ and the corresponding momentum sources, as
% %
% \begin{equation}
% \frac{D\mathbf{v}^{v}_{k}}{Dt} = \frac{\mathbf{M}^{v}_{k} + \mathbf{M}_{k}^{I_{v}}}{\alpha_k\rho_k} = C_d \frac{\mathbf{M}^{v}_{m}}{\rho_m} + (1-C_d)\frac{\mathbf{M}^{v}_{k}}{\alpha_k\rho_k}
% \end{equation}
% \begin{equation}
% \frac{D\mathbf{v}^{p}_k}{Dt} = C_d \frac{\mathbf{M}^p_m}{\rho_m} + (1-C_d)\frac{\mathbf{M}^p_k}{\alpha_k\rho_k} =\frac{\mathbf{M}^p_m}{\rho_m}\left(C_d+(1-C_d)\frac{\rho_m}{\rho_k}\right)
% \end{equation}
% %
% Advection and drift momentum sources are reasonably ignored in the calculation, which makes the pressure treatment revert to the single-phase solver. 

% The phase transport is affected by two factors, namely drift velocity and diffusion, given by
% %
% \begin{equation}
% \frac{D\alpha^i_k}{Dt} = -\sum_jV_0(\alpha^i_k\mathbf{v}^i_{mk} + \alpha^j_k\mathbf{v}^j_{mk})\cdot\nabla W_{ij}
% \end{equation}
% \begin{equation}
% \nabla^2\alpha^i_k = C_f\sum_j (\alpha^i_k-\alpha^j_k)\frac{\mathbf{x}_{ij}\cdot \nabla W_{ij}}{{\|\mathbf{x}_{ij}\|}^2 + 0.01h^2}
% \end{equation}
% %
% where $C_f$ is the diffusion coefficient; $\mathbf{x}_{ij} = \mathbf{x}_i - \mathbf{x}_j$ is the distance between the positions of particles $i$ and $j$; $h$ is the SPH support radius; $W$ is the kernel function, which is common to be cubic spline function.

% Volume incompressible SPH changes the density estimation step in DFSPH to a volume compression ratio estimation, and corrects the velocity field according to the compressibility. This has the advantage of supporting the simulation of fluids with non-uniform density fields. The volume compression ratio is computed as
% %
% \begin{equation}
% \phi_i = V_0\sum_j W_{ij}
% \end{equation}
% %
% where $V_0$ is the rest volume of a particle and $j$ denotes all neighbors of particle $i$ within the kernel radius.
% Similar to DFPSH, $\phi$ is iteratively corrected to ensure that $\phi \leq 1$.

A mixed particle $i$ with phases $k\in[1,\ldots,K]$ contains the volume fractions $\alpha^i_k$ of each phase which sum up to 1, that is
\begin{equation}
 \alpha^i = \sum_k \alpha^i_k = 1.
\end{equation}
The mixture model uses the drift velocity $\mathbf{v}^\mathrm{d}_k$ to measure the shift of the phase velocity $\mathbf{v}_k$ relative to the mixture velocity $\bar{\mathbf{v}}$ as
%
\begin{equation}
 \mathbf{v}^\mathrm{d}_k = \mathbf{v}_k - \bar{\mathbf{v}},
 \label{eq:2}
\end{equation}
where
\begin{equation}\bar{\mathbf{v}} = \sum_k \alpha_k \mathbf{v}_k.
 \label{eq:3}
\end{equation}
Finally, the particle rest density $\bar{\rho}$ is computed as
\begin{equation}
 \bar{\rho} = \sum_k \alpha_k \rho^0_k
 \label{eq:4}
\end{equation}
where $\rho^0_k$ is the rest density of phase $k$.

The recent implicit mixture model in~\cite{xu2023implicitly} is designed to improve the numerical stability of mixture models. It uses a constant rest volume of particles and adopts the pressure profile $p_k = \bar{p}$ for mixture fluids, where $p_k$ is the pressure of phase $k$ and $\bar{p}$ is the mixture pressure. The basic implicit model has five phase-level momentum sources: pressure $\mathbf{M}^\mathrm{p}_k$, viscosity $\mathbf{M}^\mathrm{v}_k$, gravity $\mathbf{M}^\mathrm{g}_k$, advection $\mathbf{M}^\mathrm{a}_k$, and interphase $\mathbf{M}^\mathrm{I}_k$; the corresponding velocity fields and mixture momenta are denoted accordingly, \emph{e.g.}, $\mathbf{v}_k^\mathrm{v}$ and $\bar{\mathbf{M}}^\mathrm{v}$ for $\mathbf{M}^\mathrm{v}_k$. In detail: $\mathbf{M}^\mathrm{a}_k$ is caused by the phase drift velocity; $\mathbf{M}^\mathrm{I}_k$ obeys a strict force balance model $\sum_k \mathbf{M}^\mathrm{I}_k=0$; according to Eqn.~\ref{eq:2}, we get $\bar{\mathbf{M}}^* = \mathbf{M}^*_k - \mathbf{M}^\mathrm{d}_k$ with $*\in\{p,v,g,a,I\}$; finally, the mixture momentum sources can be obtained by summing up all momentum sources of all phases as
\begin{equation}
 \alpha_k\rho^0_k\frac{D\mathbf{v}_k}{Dt} = \mathbf{M}^\mathrm{p}_k + \mathbf{M}^\mathrm{v}_k + \mathbf{M}^\mathrm{g}_k + \mathbf{M}^\mathrm{a}_k + \mathbf{M}^\mathrm{I}_k,
 \label{eq:5}
\end{equation}
%
\begin{equation}
 \bar{\rho}\frac{D\bar{\mathbf{v}}}{Dt} = \bar{\mathbf{M}}^\mathrm{p} + \bar{\mathbf{M}}^\mathrm{v} + \bar{\mathbf{M}}^\mathrm{g} + \bar{\mathbf{M}}^\mathrm{a} + \bar{\mathbf{M}}^\mathrm{I}.
 \label{eq:6}
\end{equation}
From Eqns.~\ref{eq:2}, \ref{eq:5}, and \ref{eq:6}, we get the change rate of phase drift velocity as
\begin{equation}
 \frac{D\mathbf{v}^\mathrm{d}_k}{Dt} = \frac{\mathbf{M}^*_k}{\alpha_k\rho^0_k} - \frac{\bar{\mathbf{M}}^*}{\bar{\rho}}.
\end{equation}

The relationship between $\mathbf{M}^\mathrm{I}_k$ and $\mathbf{M}^\mathrm{d}_k$ can be deduced from the no-drift and free-drift situations to finally get $\mathbf{M}^\mathrm{I}_k=C_d\mathbf{M}_\mathrm{k}^\mathrm{J}$, where $\mathbf{M}_k^\mathrm{J}$ is the number interphase momentum and $C_d \in [0, 1]$ is a regulatory factor: $C_d=0$ recovers free-drift, which means that the interphase momentum is zero, and $C_d=1$ recovers no-drift, which means that the phase drift velocity is zero.

Computing the momentum sources $\mathbf{v}^\mathrm{v}_{k}$ and $\mathbf{v}^\mathrm{p}_k$ uses $C_d$ and the corresponding momentum sources, as
\begin{equation}
 \frac{D\mathbf{v}^\mathrm{v}_{k}}{Dt} = C_d \frac{\bar{\mathbf{M}}^\mathrm{v}}{\bar{\rho}} + (1-C_d)\frac{\mathbf{M}^\mathrm{v}_{k}}{\alpha_k\rho^0_k},
\end{equation}
\begin{equation}
 \frac{D\mathbf{v}^\mathrm{p}_k}{Dt} = C_d \frac{\bar{\mathbf{M}}^\mathrm{p}}{\bar{\rho}} + (1-C_d)\frac{\mathbf{M}^\mathrm{p}_k}{\alpha_k\rho^0_k} =\frac{\bar{\mathbf{M}}^\mathrm{p}}{\bar{\rho}}\left(C_d+(1-C_d)\frac{\bar{\rho}}{\rho^0_k}\right).
 \label{eq:9}
\end{equation}
%
Advection and drift momentum sources are reasonably ignored in the calculation, and the pressure profile $p_k=\bar{p}$ is adopted, as above. These make the pressure treatment revert to the single-phase solver. For a detailed derivation of momentum sources, see~\cite{xu2023implicitly}.

The viscosity in the original implicit mixture model is designed to bind phases together, and the phase-level viscous momentum source is derived from the single-phase fluid viscosity
\begin{equation}
    \mathbf{M}_k^{\nu} = \alpha_k \mu_k \nabla^2 (\bar{\mathbf{v}} + \sum_j \mathbf{v}^d_kW_{ij}).
\end{equation}
This derivation is consistent with the viscosity description of the standard mixture model. Yet, the mixture momentum $\bar{\mathbf{M}}^\nu$ can be conservatively reconstructed via $\sum_k \mathbf{M}^\nu_k = \bar{\mathbf{M}}^\nu$ from the phase momentum $\mathbf{M}^\nu_k$ only when the viscosities of all phases are equal.

The phase transport is affected by two factors, namely drift velocity and diffusion, given by
%
\begin{equation}
 \frac{D\alpha^i_k}{Dt} = -\sum_jV_0(\alpha^i_k\mathbf{v}^{i,\mathrm{d}}_k + \alpha^j_k\mathbf{v}^{j,\mathrm{d}}_k)\cdot\nabla W^{ij},
 \label{eq:10}
\end{equation}
\begin{equation}
 \nabla^2\alpha^i_k = C_f\sum_j (\alpha^i_k-\alpha^j_k)\frac{\mathbf{x}^{ij}\cdot \nabla W^{ij}}{{\|\mathbf{x}^{ij}\|}^2 + 0.01h^2},
 \label{eq:11}
\end{equation}
%
where $\mathbf{v}^{i,\mathrm{d}}_k$ is $\mathbf{v}^\mathrm{d}_k$ of particle $i$; $C_f$ is the diffusion coefficient; $\mathbf{x}^{ij} = \mathbf{x}^i - \mathbf{x}^j$ is the distance between the positions of particles $i$ and $j$; $h$ is the SPH support radius; and $W$ is the kernel function, which is common to be cubic spline function.

Volume incompressible SPH changes the density estimation step in DFSPH to a volume compression ratio estimation, and corrects the velocity field according to compressibility. This has the advantage of supporting the simulation of fluids with non-uniform density fields. The volume compression ratio is computed as
%
\begin{equation}
 \phi^i = V^0\sum_j W^{ij}
\end{equation}
%
where $V^0$ is the rest volume of a particle and $j$ denotes all neighbors of particle $i$ within the kernel radius.
Similar to DFPSH, $\phi$ is iteratively corrected to ensure that $\phi \leq 1$.


\section{Proposed Method}
\label{sec:method}
In view of the shortcomings of existing single-phase solvers and the mixture model, we propose a mixture-model-based non-Newtonian fluid simulation method to simulate dynamic and highly viscoelastic fluid scenarios. The viscoelastic stress calculation is presented in Sec.~\ref{sec:conf_tensor}, the extended non-Newtonian simulation mechanism in Sec.~\ref{sec:nonnewton_mech}, and the dynamic phase transport control in Sec.~\ref{sec:b_e_network}.
%

%
\subsection{Conformation Tensor Based Viscoelastic Stress}
\label{sec:conf_tensor}
%
Polymer fluids provide one way for modeling non-Newtonian fluids. The polymer provides viscoelasticity to the fluid, and its concentration affects the lubrication of the fluid on the polymer and the friction between polymers, leading to an overall viscoelastic effect. In non-Newtonian fluid dynamics, the conformation tensor~\cite{balci2011symmetric} can be exploited to control viscoelastic stress of polymer fluids.

The polymer fluid model defines the polymer stress tensor, which is the viscoelastic stress of the fluid, as
\begin{equation}
 \mathbf{\sigma} = c\eta_\mathrm{s} \mathbf{s}(\mathbf{C}),
\label{eq:13}
\end{equation}
where $\eta_\mathrm{s}$ is the mixture viscosity; $c$ is the mixture concentration. In the mixture model, $c=\alpha_{2}$, and the symmetric positive definite polymer conformation tensor $\mathbf{C}$ evolves as 
\begin{equation}
 \frac{D\mathbf{C}}{Dt}=\mathbf{C}\nabla \mathbf{v}+(\nabla \mathbf{v})^T\mathbf{C}-\frac{1}{\lambda}\mathbf{s}(\mathbf{C}).
\end{equation}
%
where $\lambda$ is relaxation time. This model can only describe Newtonian viscoelastic fluids. To simulate non-Newtonian fluids, a nonlinear term $\beta(\mathbf{C}-\mathbf{I})\mathbf{C}$ must be added, yielding
\begin{equation}
 \frac{D\mathbf{C}}{Dt}=\mathbf{C}\nabla \mathbf{v} +(\nabla \mathbf{v})^T\mathbf{C}-\frac{1}{\lambda}\mathbf{s}(\mathbf{C}) - \beta(\mathbf{C}-\mathbf{I})\mathbf{C},
\label{eq:15}
\end{equation}
where $\beta \in [0, 1]$ is a thinning factor that regulates the fluid's shear thinning capacity (larger $\beta$ yields stronger shear thinning).

The tensor $\mathbf{s}(\mathbf{C})$ takes different forms in various non-Newtonian models. In our work, we use the Oldroyd-B case given by
\begin{equation}
 \mathbf{s}(\mathbf{C}) = \mathbf{C}-\mathbf{I}.
\end{equation}
The mixture viscoelastic stress is calculated as
\begin{equation}
 \bar{\mathbf{\sigma}} = c\eta_\mathrm{s}(\mathbf{C}-\mathbf{I}),
\label{eq:17}
\end{equation}

In the multiphase framework, the velocity field of the mixture is reconstructed by the phase velocity. We use the implicit mixture model (Sec.~\ref{sec:mixturemodel}) as a base model and replace the viscosity momentum source $\mathbf{M}^\mathrm{v}$ with the viscoelastic stress force momentum $\mathbf{M}^\mathrm{vise}$
\begin{equation}
 \mathbf{M}_k = \mathbf{M}^\mathrm{p}_k + \mathbf{M}^\mathrm{vise}_k + \mathbf{M}^\mathrm{g}_k + \mathbf{M}^\mathrm{a}_k + \mathbf{M}^\mathrm{I}_k.
\end{equation}
Following the mixture theory, we have that
\begin{equation}
 \bar{\mathbf{M}}^\mathrm{vise} = \sum_k \mathbf{M}^\mathrm{vise}_k.
\label{eq:19}
\end{equation}

In our work, we only focus on two-phase fluids made up of water ($\alpha_1$) and polymers ($\alpha_2=1-\alpha_1$). Since the conformation tensor method focuses on the mixture as a whole, the momentum updates of each phase are not explicitly given. So, when computing momenta, we do something similar to the pressure profile:
%
\begin{equation}
 \mathbf{\sigma}_k = \bar{\mathbf{\sigma}}
\end{equation}
where $\mathbf{\sigma}_k$ is the stress of phase $k$, $\bar{\mathbf{\sigma}}$ is the mixture stress. And according to Eqn.~\ref{eq:19}, we get
\begin{equation}
 \mathbf{M}^\mathrm{vise}_k = \alpha_k \bar{\mathbf{M}}^\mathrm{vise},
\end{equation}
\begin{equation}
 \bar{\mathbf{M}}^\mathrm{vise} = \nabla \cdot \bar{\mathbf{\sigma}}.
\label{eq:22}
\end{equation}
%
Our method corrects the viscous momentum of each phase simultaneously through a uniform stress and avoids the problem of momentum loss caused by different viscosities of the phases in the original implicit mixture model.

Finally, we obtain the viscoelastic phase momentum update equation
\begin{equation}
 \frac{D\mathbf{v}^\mathrm{vise}_k}{Dt} = \frac{\bar{\mathbf{M}}^\mathrm{vise}}{\bar{\rho}}\left(C_d+(1-C_d)\frac{\bar{\rho}}{\rho^0_k}\right).
\label{eq:23}
\end{equation}
This setting does not lead to the disappearance of the multiphase characteristics, and it ensures the normal phase transport and viscoelastic properties calculation.

The relaxation time $\lambda$ in Eqn.~\ref{eq:15} can affect the fluid's viscoelastic behavior. As $\lambda$ approaches infinity, the mixture will approach an elastic solid; as $\lambda$ approaches 0, the mixture will approach a pure viscous fluid. The solution's viscosity will affect the overall viscous behavior of the mixture, which differs from the general viscosity.

Another benefit of using a multiphase framework is that the fluid's local rheological properties can be computed independently. This can simulate the discontinuous shear scenario of a fluid, which is not possible with previous models (see Figure~\ref{fig:2}).

\begin{figure}[t]
\begin{center}
\includegraphics[width=\columnwidth]{SCA_PICS/solution_local.pdf}
\end{center}
\caption{Solution dynamics. Particle colors are a mixture of phase fractions -- darker colors show higher fractions of polymer phase. The fluid's local rheological properties are affected by the solution's local concentration. For a dynamic solution system, the properties of the local fluid elements may also be different if the polymer's concentration in the above three regions is different: Region-1 shows properties of pure water; Region-3 shows viscoelastic polymer properties; Region-2 is somewhere in between.}
\label{fig:2}
\end{figure}


\subsection{Non-Newtonian Mechanism}
\label{sec:nonnewton_mech}

%
In the conformation tensor model (Sec.~\ref{sec:conf_tensor}) there are already nonlinear terms that can describe the fluid's shear thinning ability. Still, the model's descriptive ability is limited to Newtonian and shear thinning. We add shear thickening capability to the model by creating a non-Newtonian mechanism, combined with the mixture model, to dynamically calculate local fluid properties.

For the three fluid scenarios described in Figure~\ref{fig:2}, we know that the key parameter affecting the local rheological properties is the solution concentration, which can be conveniently replaced by the volume fraction of the polymer phase in the mixture model. Hence, an artificial threshold can be set to adjust the stress calculation model, we use a volume fraction threshold $\alpha_0$ to handle this. 

For the shear thinning fluid, we update the thinning factor $\beta$ in Eqn.~\ref{eq:15} by
%
\begin{equation}
\beta = \beta_0\frac{\max(0, \alpha_0 - \alpha_{2})}{\alpha_0 + \epsilon},
\label{eq:24}
\end{equation}
%
where $\beta_0$ is the basic thinning factor. When $\alpha_2 < \alpha_0$, shear thinning will be stronger, and if $\alpha_2$ it will be weaker; $\alpha_0 = 0$ recovers the basic viscoelastic model and describes a Newtonian fluid; $\epsilon$ is a regularization factor used to ensure numerical stability.

For the shear thickening, we use the Cross model, a kind of non-Newtonian viscosity model\cite{cross65}, to calculate shear-related viscosity changes. The classical Cross model is formulated as 
\begin{equation}
\mu(\gamma) = \mu _\infty + (\mu_0 - \mu _\infty)\frac{1}{1+(l\gamma)^N},
\end{equation}
%
where $\gamma=\sqrt{\frac{1}{2}\mathrm{tr}(\nabla \bar{\mathbf{v}} + (\nabla \bar{\mathbf{v}})^T)^2}$ is the shear rate; $\mu_0$ and $\mu_\infty$ are limiting values of viscosity; $l$ is a scaling factor; and $N$ is a smoothing factor.

In our model, we need to consider two aspects to change the viscosity of the fluid: the change caused by the polymer concentration $\alpha_2$ and the shear rheology. 

Inspired by the classical suspension model~\cite{wyart2014discontinuous}, we describe the non-Newtonian properties of fluids by \emph{microstructures}. Instead of explicitly computing and storing microstructures, we scale the rest dynamic viscosity by establishing a nonlinear relationship between the polymer particle phase fraction and the scaling factor. Following  Royer~\cite{royer2016rheological} and Hermes~\cite{hermes2016unsteady} on the relationship between the volume fraction and the viscosity of the polymer particle component, we compute the scaling factor 
%
\begin{equation}
 \kappa^i = (1-\alpha^{i}_2)^{-n}
 \label{eq:26}
\end{equation}
%
and apply it to the solution viscosity, where $\alpha^{i}_2$ is the volume fraction of the polymer phase in particle $i$, and $n$ denotes the strength of the nonlinear relationship (set empirically in our work to $n=1.2$). The rest dynamic viscosity is computed as
\begin{equation}
 \eta^{0}_\mathrm{s} = \kappa\eta_0,
\end{equation}
where $\eta_0$ is the basic viscosity of the solution.

Finally, we compute the effective dynamic viscosity as
\begin{equation}
 \eta_\mathrm{s}(\gamma) = \eta^{0}_\mathrm{s} + (\eta_\mathrm{max} - \eta^{0}_\mathrm{s})\frac{1}{1+(l\gamma)^N}\frac{\max(0, \alpha_2-\alpha_0)}{1-\alpha_0 + \epsilon}.
\label{eq:28}
\end{equation}

The above model can be seen as two acting `intervals' which maintain the same shear properties in each; the interval's upper and lower bounds give the upper and lower limits of shear strength. Shear thinning properties generally occur at low solution concentrations, when water lubrication in the solution exceeds friction between polymer molecules. In Eqn.~\ref{eq:24}, $\frac{\max(0, \alpha_0 - \alpha_2)}{\alpha_0 + \epsilon}$ shows that when $\alpha_2$ gradually increases in the shear thinning interval, the increase of local concentration weakens the thinning effect, thus reducing the thinning factor of the nonlinear term. When the concentration rises and exceeds the threshold, the nonlinear term in Eqn.~\ref{eq:15} vanishes and activates the calculation of shear thickening. In Eqn.~\ref{eq:28}, $\frac{\max(0, \alpha_2-\alpha_0)}{1-\alpha_0 + \epsilon}$ shows that the higher the concentration, the larger the scaling factor and the higher the solution viscosity. When $\alpha_0 = 0$, we simply set $\eta_\mathrm{s} = \eta_0$.


The viscoelastic capacity of our non-Newtonian model as a function of shear is plotted in Fig.~\ref{fig:nn_curve}.

\begin{figure}[t]
\begin{center}
\includegraphics[width=0.9\columnwidth]{SCA_PICS/nonnewton_curve/curve.pdf}
\end{center}
\caption{The Viscoelasticity-ShearRate curve is divided into the shear thinning interval and the shear thickening interval, and the rheological properties are calculated dynamically through $\alpha_2$.}
\label{fig:nn_curve}
\end{figure}


\subsection{Bonding Effects Network}
\label{sec:b_e_network}

%
In the mixture model, the phase transfer step is only affected by drift velocity and diffusion.  In the implicit mixture model, $C_d$ is used to adjust the effect of mixture on the phase motion, but it is a constant. This means that, regardless of the dynamic state of the fluid, the mixture affects the phase in the same proportion. This is not true in solutions with significant bonding interactions, such as starchy water, which are easy to mix and difficult to separate, and cannot be simulated if \emph{constant} $C_d$ values are used. To alleviate this, we propose a bond cooperation network that dynamically modifies the parameter $C_d$ at each time step according to the fluid's local concentration, as follows.

For a simulation, we set a basic drag parameter $C^0_d$, 
making next $C_d$ depend on it. The core concept of the bonding effects network is \emph{obstruction}: For particle $i$, the polymer concentration carried by its neighbors hinders the momentum exchange between the phases, which causes the $C_d$ value to rise there. We use the SPH method to estimate particle $i$'s value $C_d$ (see also Figure~\ref{fig:3}) as
%
\begin{equation}
 C_d = C^0_d + (1-C^0_d)\sum^j_{j\neq i}V^j\alpha^j_2 W^{ij},
 \label{eq:29}
\end{equation}
%
where $V^j$ is the volume of particle $j$, and $\sum^j_{j\neq i}V^j\alpha^j_2 W^{ij}$ denotes the blocking factor.

\begin{figure}[htb]
\begin{center}
\includegraphics[width=0.9\columnwidth]{SCA_PICS/bonding_effects.pdf}
\end{center}
\caption{Three bonding effect cases. Red arrows show the blocking effect -- the longer the arrows, the stronger this effect. When particle $i$'s neighbors are all pure water, no shift occurs and $C^i_d=C^0_d$. When  neighbors are all pure polymer, momentum exchange blocking occurs, yielding a no-drift scenario and $C^i_d=1$. A more common scenario is a neighbor with an unsaturated phase which activates the blocking effect. Particles in the figure are drawn using pure water or pure polymer phases; the case of particles with unsaturated phases can already be described by these three examples.}
\label{fig:3}
\end{figure}


\section{Implementation}
\label{sec:implem}

\begin{table}[b]
\centering
\small
\caption{Model parameters}
\begin{tabular}{lll}
\hline
Parameter & Meaning & Range\\
\hline
$C_f$ & diffusion coefficient & [0, 1]\\
$C^0_d$ & rest drag coefficient &  [0, 1]\\
$\beta_0$ & rest shear thinning factor & [0, 1]\\
$\alpha_0$ & phase volume fraction threshold & [0, 1]\\
$\eta_f$ & rest viscosity of water & (0, 0.02)\\
$\eta_0$ & basic viscosity of solution & (0, 20)\\
$\eta_{max}$ & max viscosity of solution & (0, 20)\\
$\lambda$ & relaxation time & (0, 1)\\
\hline
\end{tabular}
\label{tab:params}
\end{table}

We implemented our model in CUDA 12.3. All experiments discussed next were run on a single NVidia RTX 3070Ti card. All relevant parameters are summarised in Table~\ref{tab:params}. The first two parameters adjust the mixing process; the latter ones control the desired non-Newtonian fluid effects. The model's elastic calculationdepends mainly on parameter $\lambda$ and viscous behavior on $\eta_f, \eta_0, \eta_{\max}$. When $\lambda \to 1$, the model is purely elastic; when $\eta \to \infty$ or $\lambda \to 0$, the model is purely viscous.

Algoritm~\ref{alg:code} shows an overview of our algorithm and its main steps. We start by setting the conformation tensor to $\mathbf{I}$ for all particles. We compute particle neighbors (here and next) by a unified grid-based neighbor search~\cite{green2010particle}. Next, the simulation loop starts, consisting of six steps.
\textbf{Step 1} updates the current state of the fluids and precomputes the viscoelastic solver related parameters. \textbf{Step 2} uses an VFSPH solver to compute the pressure momentum and next uses this to update the phase velocities and mixture velocity to keep the velocity field divergence-free. \textbf{Step 3} updates the fluid field based on gravity, surface tension forces also can be added here. \textbf{Step 4 employs our viscoelastic solver to compute viscoelastic stress forces}. \textbf{Step 5} uses VFSPH to enforce the fluid's incompressibility. Finally, \textbf{Step 6} updates the volume fractions, particle positions, and particles' neighbors.

\begin{algorithm}[t]
\centering
\small
\caption{Non-Newtonian fluid simulation algorithm}
\begin{tabular}{l}
Initialization:                                                            \\
\hspace{1em} \bfseries set $\mathbf{C}=\mathbf{I}$         \\
\hspace{1em} \bfseries Update Neighbors              \\
Loop:                                                            \\
\hspace{1em} \bfseries 1. Precomputation:         \\
\hspace{2em} \bfseries for all particles:                        \\
\hspace{3em} compute mixture velocity $\bar{\mathbf{v}}$ using Eqn.~\ref{eq:3} \\
\hspace{3em} compute phase drift velocities $\mathbf{v}^\mathrm{d}_k$ using Eqn.~\ref{eq:2} \\
\hspace{3em} compute mixture rest density $\bar{\rho}$ using Eqn.~\ref{eq:4}      \\
\hspace{3em} compute $\beta$ and $\kappa$ using Eqns.~\ref{eq:24} and \ref{eq:26}   \\
\hspace{3em} compute drag coefficient $C_d$ using Eqn.~\ref{eq:29}              \\

\hspace{1em} \bfseries 2. VFSPH divergence-free Solver:         \\
\hspace{2em} \bfseries for all particles:                        \\[-1mm]
\hspace{3em} compute pressure momenta $\mathbf{M}^{\mathrm{p}^\mathrm{div}}_{k}$ using \textbf{VFSPH}              \\
\hspace{3em} update phase velocities $\mathbf{v}_k$ using Eqn.~\ref{eq:9}              \\
\hspace{3em} recompute mixture velocity $\bar{\mathbf{v}}$ using Eqn.~\ref{eq:3} \\

\hspace{1em} \bfseries 3. Advection:         \\
\hspace{2em} \bfseries for all particles:                        \\[-1mm]
\hspace{3em} update phase velocity from $\mathbf{M}^\mathrm{g}_k$ using $\mathbf{v}_k+=\textbf{g}\Delta t$ \\
\hspace{3em} recompute mixture velocity $\bar{\mathbf{v}}$ using Eqn.~\ref{eq:3} \\

\hspace{1em} \bfseries 4. Viscoelastic Solver:         \\
\hspace{2em} \bfseries for all particles:                        \\
\hspace{3em} update conformation tensor $\mathbf{C}$ using Eqn.~\ref{eq:15}              \\
\hspace{3em} update phase velocity from $\mathbf{M}^\mathrm{vise}_{k}$ using Eqns.~\ref{eq:17} and \ref{eq:23} \\
\hspace{3em} recompute mixture velocity $\bar{\mathbf{v}}$ using Eqn.~\ref{eq:3} \\

\hspace{1em} \bfseries 5. VFSPH incompressible Solver:         \\
\hspace{2em} \bfseries for all particles:                        \\[-1mm]
\hspace{3em} compute pressure momentum $\mathbf{M}^{\mathrm{p}^\mathrm{incomp}}_{k}$ using \textbf{VFSPH}              \\
\hspace{3em} update phase velocity $\mathbf{v}_k$ using Eqn.~\ref{eq:9}        \\
\hspace{3em} recompute mixture velocity $\bar{\mathbf{v}}$ using Eqn.~\ref{eq:3} \\

\hspace{1em} \bfseries 6. Finalize:         \\
\hspace{2em} \bfseries Phase Transfer:                        \\
\hspace{3em} update volume fraction using Eqns.~\ref{eq:10} and \ref{eq:11}  \\
\hspace{2em} \bfseries for all particles:                        \\
\hspace{3em} recompute each phase drift velocity $\mathbf{v}^\mathrm{d}_k$ using Eqn.~\ref{eq:2} \\
\hspace{3em} update particle positions $\mathbf{x}^i$ using $\mathbf{x}^i\, +\!\!= \bar{\mathbf{v}}\Delta t$              \\
\hspace{2em} \bfseries Update Neighbors              \\
\end{tabular}
\label{alg:code}
\end{algorithm}


\section{Results}
\label{sec:results}
%
In this section, we present several results obtained using our proposed multiphase model. It is noteworthy that our method enables the adjustment of fluid flow using a uniform set of parameters, eliminating the need to switch between different models. All experiments discussed herein employ an explicit scheme to update the fluid's conformation tensor.

\noindent\textbf{Viscoelastic capacity:} Figure~\ref{fig:f_e} shows a set of multiphase Newtonian fluids ($\alpha_0 = 0$) with different viscosity and relaxation time, where we set $C^0_d=1$ to stabilize the rheological properties. Except for the small ball scenario, the other scenarios adopt a fixed time step of 1.5ms, which cannot be achieved by the viscosity calculation scheme in the implicit mixture model or by the existing explicit viscosity solvers, such as explicit Laplacian viscosity. In the upper part of Figure~\ref{fig:f_e}, $\eta_0$ and $\eta_\mathrm{max}$ range  in $[1, 15]$, and $\lambda$ ranges in $[0.005, 0.3]$ in the lower part. 

\begin{figure*}[ht]
\begin{center}
\includegraphics[width=\textwidth]{SCA_PICS/fluids/f_e.png}
\end{center}
\caption{Diamonds and a ball with different viscoelasticity values and \textbf{281k} particles. The timeline in the upper part shows the flow of different viscosity parameters ($\eta_0, \eta_\mathrm{max}$) with equal elastic coefficients (relaxation time $\lambda$). The first three diamond simulations in the timeline in the lower part show the flow with different elastic parameters with equal viscosity. The last small ball demonstrates the bouncing effect at high viscoelasticity.}
\label{fig:f_e}
\end{figure*}

\noindent\textbf{Non-Newtonian fluid:} Figures~\ref{fig:shear_blocks} and~\ref{fig:shear_bunny} show a shear-dependent non-Newtonian fluid. Our non-Newtonian mechanism assigns computational flows primarily by polymer phase fractions. In Figure~\ref{fig:shear_blocks}, for an intuitive comparison, we set three different types of fluids simultaneously \textbf{in one single simulation}: \textbf{Left}: Newtonian, \textbf{Middle}: shear thinning, and \textbf{Right}: shear thickening. It is worth noting that in existing non-Newtonian solvers based on specific non-Newtonian viscosity models (Cross~\cite{cross65}, Carreau~\cite{carreau1972rheological}, et al.), simulating different types of fluids requires manually changing the parameters of the non-Newtonian viscosity model, which means that different types of fluids cannot be considered in one single simulation, and more dynamic scenarios cannot be achieved. The scenario mainly examines the static non-Newtonian properties and hence we set $C^0_d=1$ to keep a homogeneous solution. Figure~\ref{fig:shear_bunny} shows a simple comparison of our model with a non-Newton solver based on the Cross model for another set of \textbf{single simulations}. This scenario examines dynamic flow effects with phase transfer, where the rheological properties of the fluid may change as the phase fraction changes. We set $C^0_d=0.8, \alpha_0 =0.65, \beta_0 =0.1$. For these two scenes, $\eta_0=2, \eta_\mathrm{max}=5$.

\begin{figure}[t]
\begin{center}
\includegraphics[width=0.85\columnwidth]{SCA_PICS/shear/shear_blocks.png}
\end{center}
\caption{A fluid block (\textbf{52k} particles) falls on an inclined plate. Left block of particles: Newtonian, Middle block: shear thinning, Right block: shear thickening. After contact with the inclined plate, the Newtonian fluid always maintains its own flow. 0--1.5s: the shear thinning fluid reduces the viscosity due to shear effect, and promotes the flow; the shear thickening fluid increases viscosity and blocks flow. 1.5--3.5s: as the shear effect weakens, the shear thinning fluid viscosity increases, and the flow slows down. The shear thickening fluid viscosity decreases, facilitating flow.}
\label{fig:shear_blocks}
\end{figure}

\begin{figure}[h]
\begin{center}
\includegraphics[width=\columnwidth]{SCA_PICS/shear/compare.png}
\end{center}
\caption{Three bunnies falling to the ground. \textbf{73k} particles. For our model, they are \textbf{Below}: newton, \textbf{Middle}: shear thinning, \textbf{Top}: shear thickening. And three same shear thinning bunnies in another scenario. In our scenario, the flow is composite and dynamic, and influenced by the polymer phase fraction, the fluid more closely resembles a viscoelastic shear thickening fluid.}
\label{fig:shear_bunny}
\end{figure}

\begin{figure}[h]
\begin{center}
\includegraphics[width=\columnwidth]{SCA_PICS/buckling/buckling.pdf}
\end{center}
\caption{The top scene shows honey poured over a donut and the bottom scene shows another buckling scene rendered as a particle state with \textbf{158k} particles. The scenes have a stable and obvious buckling effect.}
\label{fig:buckling}
\end{figure}

\noindent\textbf{Buckling effect:} The buckling effect is a complex flow behavior of highly viscous fluids, which can be correctly reproduced by our model; see Figure~\ref{fig:buckling}. We set $C^0_d=0.85, \lambda=0.0075, \alpha_0 =0.1, \beta_0 =0.1, \eta_0=10, \eta_\mathrm{max}=13$.

\noindent\textbf{Bonding effect network:} The network is mainly used to mitigate phase separation, which helps in easy to mix, hard to separate scenarios. In the original implicit mixture model, the constant $C_d$ leads to the same mixing and separating mechanism, which means that mixing and separating occur at the same rate. For scenarios like starch water, relatively easy mixing can be achieved by stirring, but we also need to maintain the mixing state for a certain amount of time, so the constant setting of $C_d$ is not applicable. We visually observe the blocking effect of the bonding network on phase separation through a simple scenario; see Figure~\ref{fig:ben}. We set volume fraction $\alpha=(0.3,0.7)$ and  $C^0_d=0.3, C_f=0.0005, \lambda=0.004, \alpha_0 =0, \eta_0=\eta_\mathrm{max}=1$.

\noindent\textbf{Mixing Comparison:} For fluids with a high viscosity ratio, mixing them stably is a challenge. In this experiment, we demonstrate the simulation of our method and IMM with a viscosity ratio of 1000 (water phase: 0.01 $Pa\cdot s$, polymer phase: 10 $Pa\cdot s$). In this scenario, we set the time step of IMM to 0.1 ms and the time step of our method to 1 ms. Our method is able to stably mix the fluid; see Fig.~\ref{fig:stir_compare}.

\begin{figure}[h]
\begin{center}
\includegraphics[width=\columnwidth]{SCA_PICS/bond_effect/comp_final.png}
\end{center}
\caption{The blocking effect of Bonding Effect Network on phase transfer (\textbf{98k} particles). We use a plane to segment the fluid to observe the internal state of the fluid. The arrows indicate distinct phase interfaces. The left shows phase separation of the original implicit mixture model, $C_d=0.3$. Our model's result with $C^0_d=0.3$ is on the right. It can be clearly seen that our model has a blocking effect on phase separation, which contributes to the scenario where the fluid achieves easy mixing and remains mixed at low $C^0_d$ values.}
\label{fig:ben}
\end{figure}

\noindent\textbf{Mix into cement:} This experiment mainly investigates the comprehensive ability of the model. The process of mixing water and sediment to form cement reflects easy mixing, difficult separation, local rheological differences and viscoelastic ability. This scene can also be extended to more dynamic scenes, such as making dough; see Figure~\ref{fig:stir}. We set $C^0_d=0.4, C_f=0.015, \lambda=0.006, \alpha_0 =0.65, \beta_0=0.2, \eta_0=6, \eta_\mathrm{max}=13$.

\noindent\textbf{"SCA2024" non-Newtonian:} This experiment demonstrates a dynamic non-Newtonian scene. We set 7 different non-Newtonian fluids in one single simulation. Different non-Newtonian fluids are obtained by setting different initial phase fractions. We set $C^0_d=0.6, C_f=0.015, \lambda=0.006, \alpha_0 =0.55, \beta_0=0.25, \eta_0=6, \eta_\mathrm{max}=10$. For the letters/numbers, we set `S' to $\alpha=(0.1, 0.9)$, `C' to $\alpha=(0.3, 0.7)$, `A' to $\alpha=(0.4, 0.6)$, the first `2' to $\alpha=(0.5, 0.5)$, `0' to $\alpha=(0.6, 0.4)$, the second `2' to $\alpha=(0.7, 0.3)$, and finally `4' to $\alpha=(0.9, 0.1)$; see Figure~\ref{fig:cake}.

\begin{figure}[h]
\begin{center}
\includegraphics[width=\columnwidth]{
SCA_PICS/stir_compare/stir_compare.png}
\end{center}
\caption{Mixing comparison (\textbf{332k particles)}. IMM on the left and our method on the right. In the experiment, the viscosity of the purple liquid column is 0.01$Pa\cdot s$, the viscosity of the rose red liquid column is 10$Pa\cdot s$, and the black one at the bottom is the stirring fan. At high speed mixing, the IMM is not stable, while our method is able to operate stably.}
\label{fig:stir_compare}
\end{figure}

\begin{figure}[h]
\begin{center}
\includegraphics[width=\columnwidth]{SCA_PICS/stir/stir.png}
\end{center}
\caption{Mix into cement (\textbf{150k} particles). The liquid column on the left is the pure water phase and the liquid column on the right is the pure polymer phase. To create the cement effect, we added a $1:2$ volume ratio of water to polymer, and set a low $C^0_d=0.4$ and a high viscosity. It can be seen that the two liquid columns can be stably mixed and maintain a high viscoelastic mixing state.}
\label{fig:stir}
\end{figure}

Table~\ref{tab:capacity} outlines our method's capabilities \emph{vs} with three other established fluid simulation solvers. Table~\ref{tab:timings} compares four viscosity scheme time steps under different viscosities. Table~\ref{tab:timings} shows that in high viscosity scenarios, our approach has significant advantages over other explicit schemes and can also include a wider viscosity range. Table~\ref{tab:timing_cost} shows the time cost of our method and IMM when stirring a high viscosity ratio fluid at a high speed. For single-step viscosity solving and overall computation, our method incurs slightly higher costs. However, since our method allows for larger time steps, the overall cost is lower, with a speedup of approximately $\times 1.97$.

\begin{table}[h]
\centering
\caption{\label{tab:capacity}Comparison of method capabilities}
\begin{threeparttable}
\resizebox{\columnwidth}{!}{%
    \begin{tabular}{cccccc}
    \hline
    Solver & \makecell{High viscosity \\ ($\Delta t \ge 1.5ms$}) & \makecell{Dynamic \\ Mixing} & \makecell{Non-Newtonian \\ fluids} & \makecell{Easy mixing but \\ separation blocked} & \makecell{Multitype fluids \\ in one simulation}\\
    \hline
    $Ours$ & \ding{52} & \ding{52} & \ding{52} & \ding{52} & \ding{52} \\
    $IMM$  & \textcolor{red}{\ding{56}} & \ding{52} & \textcolor{red}{\ding{56}} & \textcolor{red}{\ding{56}} & \textcolor{red}{\ding{56}} \\
    ${DFSPH}_{ev}$ & \textcolor{red}{\ding{56}} & \textcolor{red}{\ding{56}} & \ding{52} & \textcolor{red}{\ding{56}} & \textcolor{red}{\ding{56}} \\
    ${DFSPH}_{iv}$ & \ding{52} & \textcolor{red}{\ding{56}} & \ding{52} & \textcolor{red}{\ding{56}} & \textcolor{red}{\ding{56}} \\
    \hline
    \end{tabular}
}
\begin{tablenotes}
    \footnotesize
    \item  $IMM$: Implicit Mixture Model
    \item  ${DFSPH}_{ev}$: DFSPH with Explicit Viscosity
    \item  ${DFSPH}_{iv}$: DFSPH with Implicit Viscosity
\end{tablenotes}
\end{threeparttable}
\end{table}

\begin{table}[ht]
\centering
\footnotesize
\caption{Maximum time step $\Delta t$ (in ms) for each method in the corresponding viscosity range; $\eta$ is listed in $m^2/s$.}
\begin{tabular}{lllll}
\hline
Solver & $\eta \leq 0.001$ & $\eta \in [0.001, 1]$ & $\eta \in [1, 10]$ & $\eta \in [10, 20]$ \\
\hline
$Ours$ & $3$ & $3$ & $1.5$ & $0.1$ \\
$IMM$ & $3$ & $0.5$ & - & - \\
$DFPSH_{ev}$ & $10$ & $1$ & - & - \\
$DFPSH_{iv}$ & $10$ & $10$ & $5$ & $1$ \\

\hline
\end{tabular}
\label{tab:timings}
\end{table}

\begin{table}[ht]
\centering
\footnotesize
\caption{Timings of methods. This table lists the specific time costs of \textbf{IMM} and our method during high-speed stirring with a high viscosity ratio between phases. The viscosity of the water phase is set to 0.01 $Pa\cdot s$, while the viscosity of the polymer phase is set to 10 $Pa\cdot s$, and particle num is 318K.}
\begin{tabular}{cccc}
\hline
Solver & \makecell{Visco solver timing \\ of one step[s]} & \makecell{Timing cost of \\ one step[s]} & \makecell{Timing cost for \\ 1s animation[s]} \\
\hline
$IMM$ & $2.71\times10^{-5}$ & $0.009$ & 183.351 \\
$Ours$ & $5.94\times10^{-5}$ & $0.106$ & $93.042$ \\

\hline
\end{tabular}
\label{tab:timing_cost}
\end{table}

\section{Conclusion}
\label{sec:conclusion}
%
We proposed a new simulation technique for non-Newtonian fluids in a multiphase framework that makes use of the concept of mixtures so as to closely mimic the real behavior of fluids. Our method can adapt to different non-Newtonian fluid models and can reproduce their behavior for a wide range of viscosities. Also, our method is able to model a certain degree of elastomer behavior. Compared with general single-phase solvers with explicit viscosity and the original implicit mixture model, our method is more computationally efficient in highly viscous scenarios.

There are a few directions worth exploring. In this work, we used the explicit solution. An implicit solution of the conformation tensor method under a Lagrangian method may be able to improve the stability of the algorithm and improve its time stepping. The problem brought by the explicit solution is that numerical errors may occur under high stress, resulting in undesirable fracture behavior. This problem mostly occurs in ultra-high elastic scenes, and the resolution of the background grid may also affect the calculation. Multiscale multiphase flow simulation techniques should have higher potential for non-Newtonian fluid simulation. In our experiments, we also found that our model has the potential to be exploited in elastomer simulations as well.

%-------------------------------------------------------------------------
% bibtex
\bibliographystyle{eg-alpha-doi} 
\bibliography{main}       

% biblatex with biber
% \printbibliography       

\end{document}
