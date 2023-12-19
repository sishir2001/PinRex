{-# LANGUAGE OverloadedStrings #-}

module Main where

import Data.Array (Array, array, elems, (!), (//))
import Data.List (foldl', intercalate,groupBy,sortBy)
import Data.Function (on)
import qualified Data.Map as Map
import Data.Maybe (isJust, isNothing)
import qualified Data.Text as T
import qualified Data.Text.IO as TIO
import qualified Data.Text.Read as TR
import Utils (arrayIsEmpty, intToList, mapWithIndex)

data Node = Node {height :: Int, children :: Array Int (Maybe Node), childrenCount :: Int}

type StatePincodeDictionary = Map.Map T.Text [Int]

type StatePincodeRegexDictionary = Map.Map T.Text T.Text

type DebugMap = Map.Map T.Text Node

{-
    @param h : height of the node in the trie
    @return : an empty node with height h
-}
initNode :: Int -> Node
initNode h = Node h (array (0, 9) [(i, Nothing) | i <- [0 .. 9]]) 0

initNode' :: Int -> Array Int (Maybe Node) -> Int -> Node
initNode' = Node

-- START : Build a Trie from a list of pincodes
buildTrieFromPin :: [Int] -> Node -> Node
buildTrieFromPin [] currNode = currNode -- this case will never be reached
buildTrieFromPin (digit : digits) currNode@Node {height = h, children = children, childrenCount = childrenCount} =
  if not isLeaf
    then
      let childNode = buildTrieFromPin digits newNode
          newChildren = children // [(digit, Just childNode)] -- update the children array
       in initNode' h newChildren resChildCount
    else currNode
  where
    isLeaf = h == 6
    mChildNode = children ! digit
    resChildCount = if isNothing mChildNode then childrenCount + 1 else childrenCount
    newNode = case mChildNode of
      Nothing -> initNode (h + 1)
      Just childNode -> childNode

buildTrie :: [Int] -> Node -> Node
buildTrie [] currNode = currNode
buildTrie (pin : pins) currNode =
  buildTrie pins currNode'
  where
    currNode' = buildTrieFromPin (intToList pin) currNode

-- END : Build a Trie from a list of pincodes

transformRegexListToRegex :: Int -> [T.Text] -> T.Text
transformRegexListToRegex nodeHeight regexList
  | filteredListLength == 0 = ""
  | filteredListLength == 1 = head regexList
  | otherwise = openingBracket <> regex <> closingBracket
  where
    seperator = T.pack "|"
    isChildLeaf = nodeHeight == 4
    isRootNode = nodeHeight == 0
    openingBracket = if isChildLeaf then "[" else "("
    closingBracket = if isChildLeaf then "]" else ")"
    filteredRegexList = filter (/= "") regexList
    filteredListLength = length filteredRegexList
    regex =
      if isChildLeaf
        then T.concat filteredRegexList
        else T.intercalate seperator filteredRegexList

-- START : regex generating functions
regexFromTrie :: Int -> Node -> T.Text
regexFromTrie index Node {height = h, children = children, childrenCount = childrenCount}
  | arrayIsEmpty children = ""
  | isNoChild = T.pack $ show index
  | otherwise = T.pack (show index) <> transformRegexListToRegex h regexList
  where
    isRootNode = h == 0
    -- prepend = if isRootNode then "^" else ""
    isNoChild = childrenCount == 0
    regexList = mapWithIndex (flip helperDFS) (elems children) -- bfs is done using map
    helperDFS mNode i = case mNode of -- dfs is done using helperDFS
      Nothing -> ""
      Just childNode -> regexFromTrie i childNode

pincodeListToRegex :: [Int] -> T.Text
pincodeListToRegex pincodeList =
  regexFromTrie 0 rootNode
  where
    node = initNode 0
    rootNode = buildTrie pincodeList node -- build a trie with regex

-- pincodeListToRegex :: [Int] -> Node
-- pincodeListToRegex pincodeList =
--   buildTrie pincodeList node -- build a trie with regex
--   where
--     node = initNode 0

-- END : regex generating functions

-- START : read the input/output csv from arguments
parseLines :: T.Text -> (T.Text, Int)
parseLines line =
  let [pincodeText, state] = T.splitOn "," line
      Right (pin, _) = TR.decimal pincodeText
   in (state, pin)

addPincodeToDictionary :: StatePincodeDictionary -> (T.Text, Int) -> StatePincodeDictionary
addPincodeToDictionary dict (state, pincode) =
  let pincodeList = Map.findWithDefault [] state dict
   in Map.insert state (pincode : pincodeList) dict

createStatePincodeDictionary :: T.Text -> StatePincodeDictionary
createStatePincodeDictionary contents =
  let lines = T.lines contents
      parsedLines = map parseLines (tail lines) -- tail to remove the header
   in foldl' addPincodeToDictionary Map.empty parsedLines

createStatePincodeRegex :: StatePincodeDictionary -> StatePincodeRegexDictionary
createStatePincodeRegex = Map.map pincodeListToRegex

-- createStatePincodeRegex :: StatePincodeDictionary -> DebugMap
-- createStatePincodeRegex = Map.map pincodeListToRegex

printDebugMap :: DebugMap -> IO ()
printDebugMap debugMap = mapM_ printNodesOfSameHeight groupedNodes
  where
    nodesList = Map.elems debugMap
    sortedNodes = sortBy (compare `on` height) nodesList
    groupedNodes = groupBy ((==) `on` height) sortedNodes

    printNodesOfSameHeight :: [Node] -> IO ()
    printNodesOfSameHeight nodes = do
        putStrLn $ "Height: " ++ show (height (head nodes))
        mapM_ printNode nodes

    printNode :: Node -> IO ()
    printNode Node{height=h, children=children, childrenCount=c} = do
        let childNodes = map (\maybeNode -> if isJust maybeNode then 1 else 0) (elems children)
        putStrLn $ "Children: " ++ show childNodes

main :: IO ()
main = do
  contents <- TIO.readFile "pincodes.csv"
  let statePincodeDic = createStatePincodeDictionary contents
      statePincodeRegexDic = createStatePincodeRegex statePincodeDic
  -- printDebugMap statePincodeRegexDic
  print statePincodeRegexDic

-- END: read the input/output csv from arguments

-- TODO : test whether trie is formed right -- I think its working fine 
-- TODO : test function that generated regex from trie is right
