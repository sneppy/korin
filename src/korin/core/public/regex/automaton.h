#pragma once

#include "core_types.h"
#include "containers/list.h"
#include "containers/pair.h"
#include "containers/set.h"
#include "containers/tuple.h"
#include "containers/map.h"
#include "./regex_types.h"
#include "./state.h"
#include "./executor.h"

namespace Re
{
	template<typename> class AutomatonBuilder;
	template<typename> class AutomatonOptimizer;

	/**
	 * This class implements a non-deterministic
	 * finite automaton. This kind of automata
	 * can be used to implements regex and other
	 * similar features.
	 * 
	 * Example:
	 * ```cpp
	 * using namespace Re;
	 * Automaton<char> automaton;
	 * AutomatonBuilder<char> builder{automaton}; // Building regex `(a|b)+`
	 * 
	 * builder.beginGroup();
	 * builder.pushState<StateSymbol<char>>('a');
	 * builder.pushBranch();
	 * builder.pushState<StateSymbol<char>>('b');
	 * builder.endGroup();
	 * builder.endGroup();
	 * 
	 * automaton.acceptString("abbabba"); // True
	 * automaton.acceptString("abbacba"); // False
	 * ```
	 * 
	 * @param AlphaT type of the automaton alhabet
	 */
	template<typename AlphaT>
	class Automaton
	{		
		using StateT = StateBase<AlphaT>;
		using FindStateT = typename StateT::FindState;

		using AlphabetTraitsT = AlphabetTraits<AlphaT>;
		using AlphaSymbolT = typename AlphabetTraitsT::SymbolT;
		using AlphaStringT = typename AlphabetTraitsT::StringT;

		using ExecutorT = Executor<AlphaT>;
		using BuilderT = AutomatonBuilder<AlphaT>;
		using OptimizerT = AutomatonOptimizer<AlphaT>;

		friend ExecutorT;
		friend BuilderT;
		friend OptimizerT;
		friend Regex;
	
	protected:		
		/// State types definitions
		/// @{
		struct State
		{
			using EpsilonT = StateEpsilon<AlphaT>;
			using AnyT = StateAny<AlphaT>;
			using SymbolT = StateSymbol<AlphaT>;
			using StringT = StateString<AlphaT>;
			using RangeT = StateRange<AlphaT>;
			using LambdaT = StateLambda<AlphaT>;
			using MacroT = StateMacro<AlphaT>;
			using PositiveLookahedT = StatePositiveLookahed<AlphaT>;
			using NegativeLookahedT = StateNegativeLookahed<AlphaT>;
		};
		/// @}

	private:
		/**
		 * Allocates and constructs a new state
		 * with the given parameters
		 * 
		 * @param StateAnyT type of the state
		 * @param createStateArgs arguments
		 * 	passed to the state constructor
		 * @return ptr to created state
		 */
		template<typename StateAnyT, typename ...CreateStateArgsT>
		FORCE_INLINE StateAnyT * createState(CreateStateArgsT && ...createStateArgs)
		{
			// TODO {
			// 	I'd like to use a smart allocator
			// 	but I guess each state class should
			// 	have its own allocator
			// }
			return new (MallocObject<StateAnyT>{}.alloc()) StateAnyT{forward<CreateStateArgsT>(createStateArgs)...};
		}

		/**
		 * Destroys and deallocates state.
		 * 
		 * @param state ptr to state to
		 * 	be deallocated
		 */
		FORCE_INLINE void destroyState(StateT * state)
		{
			state->~StateT();
			MallocObject<StateT>{}.free(state);
		}
	
	public:
		/**
		 * Default constructor.
		 */
		Automaton();

		/**
		 * Destroy automaton. Deallocates
		 * all states.
		 */
		~Automaton();

		/**
		 * Returns a pointer to the start
		 * state.
		 */
		FORCE_INLINE const StateT * getStartState() const
		{
			return startState;
		}

		/**
		 * Returns true if other is the
		 * start state.
		 */
		FORCE_INLINE bool isStartState(const StateT * other) const
		{
			return FindStateT{}(other, startState) == 0;
		}

		/**
		 * Returns a pointer to the accepted
		 * state.
		 */
		FORCE_INLINE const StateT * getAcceptedState() const
		{
			return acceptedState;
		}

		/**
		 * Returns true if other is the
		 * accepted state.
		 */
		FORCE_INLINE bool isAcceptedState(const StateT * other) const
		{
			return FindStateT{}(other, acceptedState) == 0;
		}

		/**
		 * Creates an executor that execute
		 * this automaton.
		 */
		FORCE_INLINE ExecutorT createExecutor(const AlphaStringT & input) const
		{
			return ExecutorT{startState, acceptedState, input};
		}

		/**
		 * Creates and return a new builder
		 * for this automaton.
		 */
		FORCE_INLINE BuilderT createBuilder()
		{
			return BuilderT{*this};
		}

		/**
		 * Creates and returns a new optimizer
		 * for this automaton.
		 */
		FORCE_INLINE OptimizerT createOptimizer()
		{
			return OptimizerT{*this};
		}

		/**
		 * Create a new state and add to
		 * the list of allocated states.
		 * 
		 * @param StateAnyT type of the
		 * 	state
		 * @param createStateArgs arguments
		 * 	passed to the state constructor
		 * @return ptr to created state
		 */
		template<typename StateAnyT, typename ...CreateStateArgsT>
		StateAnyT * pushState(CreateStateArgsT && ...createStateArgs)
		{
			// Create state and push into
			// allocated states list
			StateAnyT * state = createState<StateAnyT>(forward<CreateStateArgsT>(createStateArgs)...);
			allocatedStates.pushBack(state);
			return state;
		}

		/**
		 * Returns true if the provided
		 * sequence of symbols is accepted
		 * by the automaton, i.e. if
		 * the automaton can consume all
		 * the input symbols and terminate
		 * on the accepted state.
		 * 
		 * Example:
		 * ```
		 * (ab)+ acceptString "abc" = false
		 * (abc)+ acceptString "abc" = true
		 * ```
		 * 
		 * @param input sequence of input
		 * 	symbols
		 */
		bool acceptString(const AlphaStringT & input) const;

		/**
		 * Returns a string representation
		 * of the automaton (depth first).
		 */
		FORCE_INLINE String toString() const
		{
			return printStateGraph(startState, acceptedState);
		}

	protected:
		/// List of allocated states
		List<StateT*> allocatedStates;

		/// Single start state
		StateT * startState;

		/// Single accepted state
		StateT * acceptedState;
	};

	template<typename AlphaT>
	Automaton<AlphaT>::Automaton()
		: allocatedStates{}
		, startState{nullptr}
		, acceptedState{nullptr}
	{
		// Create start and accepted states
		startState = createState<typename State::EpsilonT>();
		acceptedState = createState<typename State::EpsilonT>();
	}

	template<typename AlphaT>
	Automaton<AlphaT>::~Automaton()
	{
		for (StateT * state : allocatedStates)
		{
			// Destroy all allocated states
			destroyState(state);
		}

		// Destroy start and accepted states
		destroyState(startState);
		destroyState(acceptedState);
	}

	template<typename AlphaT>
	bool Automaton<AlphaT>::acceptString(const AlphaStringT & input) const
	{
		ExecutorT executor = createExecutor(input);
		
		// Step until string is either accepted
		// or rejected
		bool isAccepted;
		while (!executor.step(isAccepted));

		return isAccepted;
	}
	
	/**
	 * An helper class to build an
	 * automaton from scratch.
	 * 
	 * Example:
	 * ```
	 * AutomatonBuilder<char> builder{automaton};
	 * builder.pushState<StateSymbol<char>>('a'); // 'a'
	 * builder.pushJump(); // '+'
	 * builder.pushState<StateSymbol<char>>('b'); // 'b'
	 * builder.pushJump(); // '+'
	 * builder.endGroup(); // '\0'
	 * ```
	 * 
	 * @param AlphaT type of the
	 * 	alphabet
	 */
	template<typename AlphaT>
	class AutomatonBuilder
	{
		using AutomatonT = Automaton<AlphaT>;
		using StateT = typename AutomatonT::StateT;
		using State = typename AutomatonT::State;
		
		/**
		 * Clones current group and pushes
		 * it onto the current state.
		 */
		void cloneCurrentGroup()
		{
			using Visit = Tuple<StateT*, StateT*>;

			// Get start and end states and
			// create duplicates
			StateT * startState = groupStart[currentGroup];
			StateT * endState = groupEnd[currentGroup];

			// Keep track of to visit state
			// and already visited states
			List<Visit> visitQueue;
			Visit currVisit{startState, currentState};
			Map<StateT*, StateT*, typename StateT::FindState> visitedStates;

			do
			{
				// Get current state and previous state
				StateT * currState = currVisit.template get<0>();
				StateT * prevState = currVisit.template get<1>();

				if (!visitedStates.find(currState, currentState))
				{
					// State not already seen.
					// Duplicate state
					currentState = currState->cloneState(automaton);
					
					// Add to next states
					prevState->addNextState(currentState);

					if (currState != endState)
					{
						// If current visit state is not
						// end state, add out connections

						for (StateT * nextState : currState->getNextStates())
						{
							visitQueue.pushBack(Visit{nextState, currentState});
						}
					}

					// Add to visited states
					visitedStates.insert(currState, currentState);
				}
				else
				{
					// State already visited, just
					// add connection to prev state
					prevState->addNextState(currentState);
				}
			} while (visitQueue.popBack(currVisit));
		}

	public:
		/**
		 * Create a builder for the
		 * given automaton.
		 * 
		 * @param inAutomaton ref to
		 * 	the automaton to build
		 */
		FORCE_INLINE explicit AutomatonBuilder(AutomatonT & inAutomaton)
			: automaton{inAutomaton}
			, currentState{inAutomaton.startState}
			, groupStart{inAutomaton.startState}
			, groupEnd{inAutomaton.acceptedState}
			, currentGroup{1}
		{
			//
		}

		/**
		 * Create and push a new state onto
		 * the current state. An epsilon
		 * state is also prepended before
		 * the created state.
		 * 
		 * @param StateAnyT explicit type of
		 * 	the state to be created
		 * @param createStateArgs arguments
		 * 	passed to the state constructor
		 * @return ref to self
		 */
		template<typename StateAnyT, typename ...CreateStateArgsT>
		FORCE_INLINE AutomatonBuilder & pushState(CreateStateArgsT && ...createStateArgs)
		{
			// Create state and epsilon node
			// to create temporary group
			StateT * epsilon = automaton.template pushState<typename State::EpsilonT>();
			StateT * state = automaton.template pushState<StateAnyT>(forward<CreateStateArgsT>(createStateArgs)...);

			// Link to current state and set current
			currentState->addNextState(epsilon)->addNextState(state);
			currentState = state;

			// Create temporary groups
			groupStart[currentGroup] = epsilon;
			groupEnd[currentGroup] = state;

			return *this;
		}

		/**
		 * Begins a new group (i.e. opens
		 * a parenthesis in regex syntax).
		 * 
		 * @return ref to self
		 */
		FORCE_INLINE AutomatonBuilder & beginGroup()
		{
			// Push new epsilon state for start
			// group and end group states
			StateT * startState = automaton.template pushState<typename State::EpsilonT>();
			StateT * endState = automaton.template pushState<typename State::EpsilonT>();

			currentState->addNextState(startState);
			currentState = startState;

			if (currentGroup < maxNumGroups - 1)
			{
				groupStart[currentGroup] = startState;
				groupEnd[currentGroup] = endState;
				currentGroup += 1;
			}
			else ; // TODO: Error

			return *this;
		}

		/**
		 * Ends innermost group (i.e. closes
		 * innermost parenthesis in regex
		 * syntax).
		 * 
		 * @return ref to self
		 */
		FORCE_INLINE AutomatonBuilder & endGroup()
		{
			if (currentGroup > 0)
			{
				StateT * endState = groupEnd[currentGroup - 1];

				currentState->addNextState(endState);
				currentState = endState;

				currentGroup -= 1;	
			}
			else ; // TODO: Error

			return *this;
		}
		
		/**
		 * Begins a new macro state group.
		 * 
		 * @param MacroAnyT type of the
		 * 	macro state
		 * @return ref to self
		 */
		template<typename MacroAnyT>
		FORCE_INLINE AutomatonBuilder & beginMacro()
		{
			static_assert(IsBaseOf<typename State::MacroT, MacroAnyT>::value, "MacroT must be a derived type of StateMacro");

			// Create start and accepted state for macro
			StateT * startState = automaton.template pushState<typename State::EpsilonT>();
			StateT * acceptedState = automaton.template pushState<typename State::EpsilonT>();

			// Push macro state
			pushState<MacroAnyT>(startState, acceptedState);

			// Create macro frame
			// TODO: Check max groups
			currentGroup += 2;
			groupStart[currentGroup - 1] = startState;
			groupEnd[currentGroup - 1] = acceptedState;

			// Set start state as current state
			currentState = startState;

			return *this;
		}

		/**
		 * Ends current macro group.
		 * 
		 * @return ref to self
		 */
		FORCE_INLINE AutomatonBuilder & endMacro()
		{
			// TODO: Check we are inside macro
			// Terminate macro automaton
			endGroup();

			// Pop macro frame and set macro state as current
			currentGroup -= 1;
			currentState = groupEnd[currentGroup];

			return *this;
		}

		/**
		 * Commits current sequence in group as
		 * a branch (i.e. like a pipe `|` in
		 * regex syntax).
		 * 
		 * Example:
		 * ```
		 * before: B->1->2->(3)->| E
		 * 
		 * after: (B)->1->2->3->E
		 *          \->|
		 * ```
		 * 
		 * @return ref to self
		 */
		FORCE_INLINE AutomatonBuilder & pushBranch()
		{
			// Terminate and push current branch
			// by linking current state to end
			// of current group.
			currentState->addNextState(groupEnd[currentGroup - 1]);
			currentState = groupStart[currentGroup - 1];

			return *this;
		}

		/**
		 * Creates a shortcut, similar to
		 * a branch, that skips the current
		 * group (equivalent to star `*`
		 * operation in regex syntax).
		 * 
		 * Example:
		 * ```
		 * before: B->1->2->(3)->| E
		 * 
		 * after: B->1->2->3->(e)->|
		 *        \___________^
		 * ```
		 * 
		 * Note that the same effect can
		 * be achieved using a combination
		 * of group and branch. However that
		 * would not be possible for a single
		 * state (i.e. the single state would
		 * need to be inside a proper group).
		 * 
		 * Example:
		 * ```
		 * before: e->(1)->|
		 * 
		 * after: e->1->(e)->|
		 *        \_____^
		 * ```
		 * 
		 * @return ref to self
		 */
		FORCE_INLINE AutomatonBuilder & pushSkip()
		{
			// We need an epsilon state to
			// skip the current group
			StateT * epsilon = automaton.template pushState<typename State::EpsilonT>();
			
			currentState->addNextState(epsilon);
			currentState = epsilon;

			// Create shortcut
			groupStart[currentGroup]->addNextState(epsilon);

			return *this;
		}

		/**
		 * Creates an unconditional jump from
		 * the end of the current group to
		 * the beginning of the same group
		 * (it's equivalent to the plus `+`
		 * symbol in regex syntax).
		 * 
		 * Example:
		 * ```
		 * before: B->1->2->3->(E)->|
		 * 
		 * after: B->1->2->3->(E)->|
		 *        ^\__________|
		 * ```
		 * 
		 * @return ref to self
		 */
		FORCE_INLINE AutomatonBuilder & pushJump()
		{
			// Jump from end of current group
			// to the beginning of the group
			groupEnd[currentGroup]->addNextState(groupStart[currentGroup]);

			return *this;
		}

		/**
		 * Repeats the last inserted state or
		 * group @c n times (equivalent to
		 * `{n}` in regex syntax).
		 * 
		 * Example:
		 * ```
		 * before: e->(1)->|
		 * 
		 * after: e->1->e->1->e->(1)->|
		 * ```
		 * 
		 * @param minRepeats minimum number
		 * 	of group repetitions (default 1)
		 * @param maxRepeats maximum number
		 * 	of group repetitions. If this
		 * 	value is zero, @c minRepeats is
		 * 	treated as `at least` lower
		 * 	bound
		 * @return ref to self
		 */
		FORCE_INLINE AutomatonBuilder & pushRepeat(int32 minRepeats = 1, int32 maxRepeats = 1)
		{
			// Create epsilon state as future group end
			// and also to skip groups
			StateT * epsilon = automaton.template pushState<typename State::EpsilonT>();
			StateT * prevState;

			for (int32 repeatIdx = 1; repeatIdx < minRepeats; ++repeatIdx)
			{
				// Process min repetitions first.
				// Insert an epsilon state between
				// clones so that we can create a
				// jump circuit if necessary
				// (otherwise we have problems
				// with symbol states vs. proper
				// groups)
				prevState = currentState = currentState->addNextState(automaton.template pushState<typename State::EpsilonT>());
				
				// Create a clone of the group
				cloneCurrentGroup();
			}

			if (maxRepeats == 0)
			{
				// Create jump circuit
				currentState->addNextState(prevState);
			}
			else
			{
				CHECK(minRepeats <= maxRepeats);

				for (int32 repeatIdx = minRepeats; repeatIdx < maxRepeats; ++repeatIdx)
				{
					// Groups here can be skipped.
					currentState->addNextState(epsilon);

					// Create group clone
					cloneCurrentGroup();
				}
			}

			// Define current group as entire
			// repetition group, such that
			// following operations address
			// it
			groupEnd[currentGroup] = currentState = currentState->addNextState(epsilon);

			return *this;
		}

	protected:
		/// Max number of groups
		static constexpr sizet maxNumGroups = 127;

		/// Ref to building automaton
		AutomatonT & automaton;

		/// Current active state
		StateT * currentState;

		/// Groups buffers
		/// @{
		StateT * groupStart[maxNumGroups];
		StateT * groupEnd[maxNumGroups];
		/// @}

		/// Current active group
		int8 currentGroup;
	};

	/**
	 * 
	 */
	template<typename AlphaT>
	class AutomatonOptimizer
	{
		using AutomatonT = Automaton<AlphaT>;
		using StateT = typename AutomatonT::StateT;
		using State = typename AutomatonT::State;

	public:
		/**
		 * Creates a new optimizer for
		 * the given automaton.
		 * 
		 * @param inAutomaton the
		 * 	automaton to be optimized
		 */
		FORCE_INLINE AutomatonOptimizer(AutomatonT & inAutomaton)
			: automaton{inAutomaton}
		{
			//
		}

		/**
		 * 
		 */
		AutomatonOptimizer & removeEpsilons();

	protected:
		/// The automaton to be optimized
		AutomatonT & automaton;
	};

	template<typename AlphaT>
	AutomatonOptimizer<AlphaT> & AutomatonOptimizer<AlphaT>::removeEpsilons()
	{
		for (StateT * state : automaton.allocatedStates)
		{
			if (auto * epsilon = state->template as<typename State::EpsilonT>())
			{
				if (epsilon->getPrevStates().getCount() == 1 && epsilon->getNextStates().getCount() > 0)
				{
					// Merge into previous state
					epsilon->mergePrevState();
					// TODO: Dealloc state
				}
				else if (epsilon->getNextStates().getCount() == 1 && epsilon->getPrevStates().getCount() > 0)
				{
					// Merge into next state
					epsilon->mergeNextState();
					// TODO: Dealloc state
				}
			}
		}

		return *this;
	}
} // namespace Re
